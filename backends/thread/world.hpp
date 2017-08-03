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
#include <bulk/messages.hpp>
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
    registered_variable() : buffer(0), receiveBuffer(0), size(0) {}
    ~registered_variable() {}

    void* buffer;        // Local copy, stored in var::impl
    char* receiveBuffer; // Fixed size receive buffer, allocated by world
    size_t size;         // Size of the allocated receive buffer
};

struct registered_queue {
    registered_queue() : base(0), mutex(new std::mutex()) {}
    ~registered_queue() { delete mutex; }

    class queue_base* base;          // Local queue, stored in queue::impl
    std::vector<char> receiveBuffer; // Dynamic sized receive buffer
    std::mutex* mutex;                // Sending mutex
};

// single `world_state` instance shared by every thread
class world_state {
  public:
    explicit world_state(int processors) : sync_barrier(processors) {
        variables_.reserve(20 * processors);
        queues_.reserve(20 * processors);
    }

    barrier sync_barrier;

    std::mutex var_mutex;
    std::vector<registered_variable> variables_;

    std::mutex queue_mutex;
    std::vector<registered_queue> queues_;

    std::mutex log_mutex; // mutex for the vector and for sending output
    std::vector<std::pair<int, std::string>> logs;
    std::function<void(int, const std::string&)> log_callback;
};

// separate `world` instance for every thread
class world : public bulk::world {
  public:
    world(world_state* state, int pid, int nprocs)
        : state_(state), pid_(pid), nprocs_(nprocs) {}
    ~world() {}

    // Needs a move for environment::spawn to work
    world(world&& other) {
        state_ = other.state_;
        pid_ = other.pid_;
        nprocs_ = other.nprocs_;
        get_tasks_ = std::move(other.get_tasks_);
        put_tasks_ = std::move(other.put_tasks_);
    }

    int active_processors() const override { return nprocs_; }
    int processor_id() const override { return pid_; }

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
        for (auto& t : get_tasks_) {
            memcpy(t.dst, t.src, t.size);
        }
        get_tasks_.clear();

        barrier();

        // Puts from this processor
        for (auto& t : put_tasks_) {
            memcpy(t.dst, t.src, t.size);
        }
        put_tasks_.clear();

        // Queue messages to this processor
        auto& qs = state_->queues_;
        for (auto i = 0u; i < qs.size(); i += nprocs_) {
            auto& rq = qs[i + pid_];
            if (rq.base) { // If this is a registered queue
                // If size is 0 then get_buffer_ will clear the queue
                // automatically
                auto size = rq.receiveBuffer.size();
                void* dest = rq.base->get_buffer_(size);
                memcpy(dest, rq.receiveBuffer.data(), size);
                rq.receiveBuffer.clear();
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

    template <typename... Ts>
    void log_direct(const char* format, const Ts&... ts) {
        size_t size = snprintf(0, 0, format, ts...);
        char* buffer = new char[size + 1];
        snprintf(buffer, size + 1, format, ts...);
        std::string logmessage(buffer);
        delete[] buffer;
        {
            std::lock_guard<std::mutex> lock{state_->log_mutex};
            if (state_->log_callback == nullptr)
                std::cout << logmessage << '\n' << std::flush;
            else
                state_->log_callback(pid_, logmessage);
        }
    }

    void abort() override {
        // TODO
        return;
    }

  protected:
    int register_location_(void* location, std::size_t size) override {
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
        vars[id + pid_].buffer = location;
        vars[id + pid_].receiveBuffer = new char[size];
        vars[id + pid_].size = size;
        barrier();
        return id;
    }

    void unregister_location_(int id) override {
        // No mutex needed because each thread sets a different value to zero
        // and other than that, the vector is not modified
        auto& var = state_->variables_[id + pid_];
        if (var.receiveBuffer)
            delete[] var.receiveBuffer;
        var.buffer = 0;
        var.receiveBuffer = 0;
        var.size = 0;
        return;
    }

    registered_variable& get_var_(int id, int pid) const {
        return state_->variables_[id + pid];
    }
    void* get_location_(int id, int pid) const {
        return state_->variables_[id + pid].buffer;
    }

    void put_(int processor, const void* value, std::size_t size,
              int var_id) override {
        auto& v = get_var_(var_id, processor);
        if (size != v.size) {
            log("BULK ERROR: put out of bounds");
            return;
        }
        memcpy(v.receiveBuffer, value, size);
        put_tasks_.push_back({v.buffer, v.receiveBuffer, size});
        return;
    }

    // Offset and count are number of elements
    // Size is size per element
    void put_(int processor, const void* values, std::size_t size, int var_id,
              std::size_t offset, std::size_t count) override {
        auto& v = get_var_(var_id, processor);
        if (size * (offset + count) > v.size) {
            log("BULK ERROR: array put out of bounds");
            return;
        }
        memcpy(v.receiveBuffer + size * offset, values, size * count);
        put_tasks_.push_back({(char*)v.buffer + size * offset,
                              v.receiveBuffer + size * offset, size * count});
        return;
    }

    void get_(int processor, int var_id, std::size_t size,
              void* target) override {
        get_tasks_.push_back({target, get_location_(var_id, processor), size});
        return;
    }
    // Size is per element
    void get_(int processor, int var_id, std::size_t size, void* target,
              std::size_t offset, std::size_t count) override {
        get_tasks_.push_back(
            {target, (char*)get_location_(var_id, processor) + size * offset,
             size * count});
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
        q.receiveBuffer.clear();
        return;
    }

    void send_(int processor, int queue_id, const void* data,
               std::size_t size) override {
        auto& q = get_queue_(queue_id, processor);
        std::lock_guard<std::mutex> lock{*q.mutex};
        auto& vec = q.receiveBuffer;
        vec.insert(vec.end(), (const char*)data, (const char*)data + size);
        return;
    }

    // data consists of both tag and content. size is total size.
    void send_many_(int processor, int queue_id, const void* data, size_t size,
                    int count, const void* other,
                    size_t size_of_other) override {
        static_assert(false, "`send_many_` not implemented in thread backend");
    }

    char* send_buffer_(int target, int queue_id, size_t buffer_size) override {
        static_assert(false, "`send_buffer_` not implemented in thread backend");
        return nullptr;
    }

  private:
    // This should be a reference but we can not assign it in the constructor
    // because `world` does not have a constructor. FIXME: Change this ?
    world_state* state_;
    int pid_;
    int nprocs_;

    struct copy_task {
        void* dst;
        void* src;
        size_t size;
    };
    std::vector<copy_task> get_tasks_;
    std::vector<copy_task> put_tasks_;
};

} // namespace bulk::thread
