#pragma once
#include <algorithm>
#include <condition_variable>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <functional>
#include <iostream>
#include <limits>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "barrier.hpp"
#include <bulk/future.hpp>
#include <bulk/messages.hpp>
#include <bulk/variable.hpp>
#include <bulk/world.hpp>

// Mutexes need to be shared, i.e. single instance of the class
// that is shared amongst threads.
// There is however a separate `world` object for every thread
// Therefore we create an extra class `world_state` of which there
// is only a single instance.

namespace bulk::thread {

// FIXME: receive buffer could be only partially written to
// i.e. sliced write in a coarray
struct registered_variable {
    registered_variable() : base(0), buffer(0), receiveBuffer(0), size(0) {}
    ~registered_variable() {}

    class var_base* base;
    void* buffer;        // Local copy, stored in var::impl
    char* receiveBuffer; // Fixed size receive buffer, allocated by world
    size_t capacity;     // Size of the allocated receive buffer, can change
    size_t size;         // Current filling of receiveBuffer
};

typedef std::pair<char*, size_t> sized_buffer;

struct registered_queue {
    registered_queue() : base(0), mutex(new std::mutex()) {}
    ~registered_queue() { delete mutex; }

    class queue_base* base; // Local queue, stored in queue::impl
    std::vector<sized_buffer> receiveBuffers; // Dynamic sized receive buffer
    std::mutex* mutex;                        // Mutex on receiveBuffers
};

// single `world_state` instance shared by every thread
template <typename Barrier = spinning_barrier>
class world_state {
  public:
    explicit world_state(int processors) : sync_barrier(processors) {
        variables_.reserve(20 * processors);
        queues_.reserve(20 * processors);
    }

    Barrier sync_barrier;

    std::mutex var_mutex;
    std::vector<registered_variable> variables_;

    std::mutex queue_mutex;
    std::vector<registered_queue> queues_;

    std::mutex log_mutex; // mutex for the vector and for sending output
    std::vector<std::pair<int, std::string>> logs;
    std::function<void(int, const std::string&)> log_callback;
};

// separate `world` instance for every thread
template <typename Barrier = spinning_barrier>
class world : public bulk::world {
  public:
    world(world_state<Barrier>* state, int pid, int nprocs)
    : state_(state), pid_(pid), nprocs_(nprocs) {}
    ~world() {}

    // Needs a move for environment::spawn to work
    world(world&& other) {
        state_ = other.state_;
        pid_ = other.pid_;
        nprocs_ = other.nprocs_;
        var_get_tasks_ = std::move(other.var_get_tasks_);
        var_put_tasks_ = std::move(other.var_put_tasks_);
        coarray_get_tasks_ = std::move(other.coarray_get_tasks_);
        coarray_put_tasks_ = std::move(other.coarray_put_tasks_);
    }

    int active_processors() const override { return nprocs_; }
    int rank() const override { return pid_; }

    void barrier() override { state_->sync_barrier.wait(); }

    void sync() override {
        barrier();

        // core A: x = 5;
        // core B: put(A, 18, x);
        // core C: get(x, A);
        // We guarantee that C gets the value 5 and not 18
        // so we require an extra barrier here after the gets
        // TODO: Can we somehow avoid this by turning all gets into
        // puts when the get is performed?

        // Gets to this processor
        for (auto& t : coarray_get_tasks_) {
            memcpy(t.dst, t.src, t.size);
        }
        coarray_get_tasks_.clear();

        // TODO: optimize the redundant buffer away
        for (auto& t : var_get_tasks_) {
            auto size = t.var->serialized_size();
            char* buffer = new char[size];
            t.var->serialize(buffer);
            t.future->deserialize_get(size, buffer);
            delete[] buffer;
        }
        var_get_tasks_.clear();

        barrier();

        // Puts from this processor
        for (auto& t : coarray_put_tasks_) {
            memcpy(t.dst, t.src, t.size);
        }
        coarray_put_tasks_.clear();

        for (auto& t : var_put_tasks_) {
            // we have written something to receiveBuffer earlier
            // now copy it to the var itself
            t->base->deserialize_put(t->size, t->receiveBuffer);
        }
        var_put_tasks_.clear();

        // Queue messages to this processor
        auto& qs = state_->queues_;
        for (auto i = 0u; i < qs.size(); i += nprocs_) {
            auto& rq = qs[i + pid_];
            if (rq.base) { // If this is a registered queue
                rq.base->clear_();
                for (auto& p : rq.receiveBuffers) {
                    rq.base->deserialize_push(p.second, p.first);
                    delete[] p.first;
                }
                rq.receiveBuffers.clear();
            }
        }

        // Print any log messages
        if (pid_ == 0) {
            auto& logs = state_->logs;
            // Sort on pid
            // Leave message order intact
            std::stable_sort(logs.begin(), logs.end(), [](auto& m1, auto& m2) {
                return m1.first < m2.first;
            });
            if (state_->log_callback == nullptr) {
                for (auto& log : logs) {
                    std::cout << log.second << '\n';
                }
                std::cout << std::flush;
            } else {
                for (auto& log : logs) {
                    state_->log_callback(log.first, log.second);
                }
            }
            logs.clear();
        }
        barrier();
    }

    void log_(std::string message) override {
        std::lock_guard<std::mutex> lock{state_->log_mutex};
        state_->logs.push_back(std::make_pair(pid_, message));
    }

    void abort() override {
        // TODO
        return;
    }

  protected:
    int register_variable_(class var_base* varbase) override {
        auto [location, size] = varbase->location_and_size();

        auto& vars = state_->variables_;
        int id = -1;
        {
            // The lock is not for accessing [i + pid]
            // but for the resizing that might happen
            std::lock_guard<std::mutex> lock{state_->var_mutex};
            for (auto i = 0u; i < vars.size(); i += nprocs_) {
                if (vars[i + pid_].buffer == 0) {
                    id = i;
                    break;
                }
            }
            if (id == -1) {
                id = vars.size();
                // There was no slot yet. In that case, this thread is the first
                // to reach this point, so we have to allocate `nprocs_` extra
                // slots
                vars.resize(vars.size() + nprocs_);
            }
        }
        vars[id + pid_].base = varbase;
        vars[id + pid_].buffer = location;
        vars[id + pid_].receiveBuffer = new char[size];
        vars[id + pid_].capacity = size;
        vars[id + pid_].size = 0;
        barrier();
        return id;
    }

    void unregister_variable_(int id) override {
        // No mutex needed because each thread sets a different value to zero
        // and other than that, the vector is not modified
        auto& var = state_->variables_[id + pid_];
        if (var.receiveBuffer)
            delete[] var.receiveBuffer;
        var.base = 0;
        var.buffer = 0;
        var.receiveBuffer = 0;
        var.capacity = 0;
        var.size = 0;
        return;
    }

    int register_future_(class future_base*) override { return 0; }
    void unregister_future_(class future_base*) override {}

    registered_variable& get_var_(int id, int pid) const {
        return state_->variables_[id + pid];
    }
    void* get_location_(int id, int pid) const {
        return state_->variables_[id + pid].buffer;
    }

    char* put_buffer_(int processor, int var_id, size_t size) override {
        auto& v = get_var_(var_id, processor);
        // reallocate if buffer is not the right size
        if (size > v.capacity) {
            delete[] v.receiveBuffer;
            v.receiveBuffer = new char[size];
            v.capacity = size;
        }
        v.size = size;
        var_put_tasks_.push_back(&v);
        return v.receiveBuffer;
    }

    // Offset and count are number of elements
    // Size is size per element
    void put_(int processor,
              const void* values,
              std::size_t size,
              int var_id,
              std::size_t offset,
              std::size_t count) override {
        auto& v = get_var_(var_id, processor);
        if (size * (offset + count) > v.capacity) {
            log("BULK ERROR: array put out of bounds");
            return;
        }
        memcpy(v.receiveBuffer + size * offset, values, size * count);
        coarray_put_tasks_.push_back({(char*)v.buffer + size * offset,
                                      v.receiveBuffer + size * offset, size * count});
        return;
    }

    void get_buffer_(int processor, int var_id, class future_base* future) override {
        var_get_tasks_.push_back({future, get_var_(var_id, processor).base});
        return;
    }
    // Size is per element
    void get_(int processor,
              int var_id,
              std::size_t size,
              void* target,
              std::size_t offset,
              std::size_t count) override {
        coarray_get_tasks_.push_back(
        {target, (char*)get_location_(var_id, processor) + size * offset, size * count});
        return;
    }

    int register_queue_(class queue_base* q) override {
        auto& qs = state_->queues_;
        int id = -1;
        {
            // The lock is not for accessing [i + pid]
            // but for the resizing that might happen
            std::lock_guard<std::mutex> lock{state_->queue_mutex};
            for (auto i = 0u; i < qs.size(); i += nprocs_) {
                if (qs[i + pid_].base == 0) {
                    id = i;
                    break;
                }
            }
            if (id == -1) {
                id = qs.size();
                // There was no slot yet. In that case, this thread is the first
                // to reach this point, so we have to allocate `nprocs_` extra
                // slots
                qs.resize(qs.size() + nprocs_);
            }
        }
        qs[id + pid_].base = q;
        barrier();
        return id;
    }

    registered_queue& get_queue_(int id, int pid) const {
        return state_->queues_[id + pid];
    }

    void unregister_queue_(int id) override {
        auto& q = get_queue_(id, pid_);
        q.base = 0;
        for (auto& p : q.receiveBuffers) {
            delete[] p.first;
        }
        q.receiveBuffers.clear();
        return;
    }

    char* send_buffer_(int processor, int queue_id, size_t size) override {
        char* buffer = new char[size];
        // Lock mutex only to append the pointer to vector
        auto& q = get_queue_(queue_id, processor);
        {
            std::lock_guard<std::mutex> lock{*q.mutex};
            q.receiveBuffers.push_back(std::make_pair(buffer, size));
        }
        return buffer;
    }

  private:
    // This should be a reference but we can not assign it in the constructor
    // because `world` does not have a constructor. FIXME: Change this ?
    world_state<Barrier>* state_;
    int pid_;
    int nprocs_;

    // Task for data that needs no serialization
    struct copy_task {
        void* dst;
        void* src;
        size_t size;
    };
    // Task for data that needs serialization
    struct get_task {
        class future_base* future; // dst
        class var_base* var;       // src
    };

    std::vector<get_task> var_get_tasks_;
    std::vector<registered_variable*> var_put_tasks_;
    std::vector<copy_task> coarray_get_tasks_;
    std::vector<copy_task> coarray_put_tasks_;
};

} // namespace bulk::thread
