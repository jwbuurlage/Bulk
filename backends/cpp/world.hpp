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

#include <bulk/messages.hpp>
#include <bulk/world.hpp>

// Mutexes need to be shared, i.e. single instance of the class
// that is shared amongst threads.
// There is however a separate `world` object for every thread
// Therefore we create an extra class `world_state` of which there
// is only a single instance.

namespace bulk {
namespace cpp {

// Taken from
// http://stackoverflow.com/questions/24465533/implementing-boostbarrier-in-c11
class barrier {
  public:
    explicit barrier(std::size_t count)
        : threshold_(count), count_(count), generation_(0) {}

    void wait() {
        auto gen = generation_;
        std::unique_lock<std::mutex> lock{mutex_};
        if (!--count_) {
            generation_++;
            count_ = threshold_;
            cond_.notify_all();
        } else {
            cond_.wait(lock, [this, gen] { return gen != generation_; });
        }
    }

  private:
    std::mutex mutex_;
    std::condition_variable cond_;
    std::size_t threshold_;
    std::size_t count_;
    std::size_t generation_;
};

struct queue_helper {
    queue_helper() {}
    ~queue_helper() {}

    class queue_base* base;
    int sync_id;
    std::vector<char> receiveBuffer;
    std::mutex mutex; // sending mutex
};

// single `world_state` instance shared by every thread
class world_state {
  public:
    explicit world_state(int processors) : sync_barrier(processors) {
        locations_.reserve(20 * processors);
    }

    barrier sync_barrier;
    // Used in var and coarray creation mechanism
    void* var_pointer_;

    std::mutex location_mutex;
    std::vector<void*> locations_;

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
        sync_operations_ = std::move(other.sync_operations_);
    }

    int active_processors() const override { return nprocs_; }
    int processor_id() const override { return pid_; }

    void barrier() override { state_->sync_barrier.wait(); }

    void sync() override {
        barrier();

        // Perform operations required at each sync like
        // swapping message queues
        for (auto& op : sync_operations_) {
            if (op)
                op();
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

    int register_sync_operation_(std::function<void(void)> f) {
        for (size_t i = 0; i < sync_operations_.size(); ++i) {
            if (sync_operations_[i] == nullptr) {
                sync_operations_[i] = f;
                return i;
            }
        }
        sync_operations_.push_back(f);
        return sync_operations_.size() - 1;
    }

    void unregister_sync_operation_(int id) { sync_operations_[id] = nullptr; }

    // Communication mechanism to communicate single pointers
    // Used by var and coarray constructors
    // We could use the variable-id system instead but this seems simpler.
    template <typename T>
    void set_pointer_(T* value) {
        state_->var_pointer_ = value;
    }
    template <typename T>
    T* get_pointer_() {
        return (T*)(state_->var_pointer_);
    }

  protected:
    int register_location_(void* location) override {
        int id = -1;
        {
            // The lock is not for accessing [i + pid]
            // but for the resizing that might happen
            std::lock_guard<std::mutex> lock{state_->location_mutex};
            auto& locs = state_->locations_;
            for (auto i = 0u; i < locs.size(); i += nprocs_) {
                if (locs[i + pid_] == 0) {
                    locs[i + pid_] = location;
                    id = i;
                    break;
                }
            }
            if (id == -1) {
                id = locs.size();
                // There was no slot yet. In that case, this thread is the first
                // to reach this point, so we have to allocate `nprocs_` extra
                // slots
                locs.insert(locs.end(), nprocs_, 0);
                locs[id + pid_] = location;
            }
        }
        barrier();
        return id;
    }

    void unregister_location_(int id) override {
        // No mutex needed because each thread sets a different value to zero
        // and other than that, the vector is not modified
        state_->locations_[id + pid_] = 0;
    }

    void* get_location_(int id, int pid) const {
        return state_->locations_[id + pid];
    }

    void put_(int processor, const void* value, std::size_t size,
              int var_id) override {
        memcpy(get_location_(var_id, processor), value, size);
        return;
    }

    // Offset and count are number of elements
    // Size is size per element
    void put_(int processor, const void* values, std::size_t size, int var_id,
              std::size_t offset, int count) override {
        memcpy((char*)get_location_(var_id, processor) + size * offset, values,
               size * count);
        return;
    }
    void get_(int processor, int var_id, std::size_t size,
              void* target) override {
        memcpy(target, get_location_(var_id, processor), size);
        return;
    }
    // Size is per element
    void get_(int processor, int var_id, std::size_t size, void* target,
              std::size_t offset, int count) override {
        memcpy(target, (char*)get_location_(var_id, processor) + size * offset,
               size * count);
        return;
    }

    int register_queue_(class queue_base* q) override {
        queue_helper* helper = new queue_helper;
        helper->base = q;

        helper->sync_id = register_sync_operation_([helper]() {
            // If size is 0 then get_buffer_ will clear the queue automatically
            auto size = helper->receiveBuffer.size();
            void* dest = helper->base->get_buffer_(size);
            memcpy(dest, helper->receiveBuffer.data(), size);
            helper->receiveBuffer.clear();
        });

        return register_location_(helper);
    }

    queue_helper* get_queue_(int id, int pid) const {
        return (queue_helper*)get_location_(id, pid);
    }

    void unregister_queue_(int id) override {
        queue_helper* helper = get_queue_(id, pid_);
        unregister_sync_operation_(helper->sync_id);
        delete helper;
        unregister_location_(id);
        return;
    }

    void send_(int processor, int queue_id, const void* data,
               std::size_t size) override {
        auto q = get_queue_(queue_id, processor);
        std::lock_guard<std::mutex> lock{q->mutex};
        auto& vec = q->receiveBuffer;
        vec.insert(vec.end(), (const char*)data, (const char*)data + size);
        return;
    }

  private:
    // This should be a reference but we can not assign it in the constructor
    // because `world` does not have a constructor. FIXME: Change this ?
    world_state* state_;
    int pid_;
    int nprocs_;
    std::vector<std::function<void(void)>> sync_operations_;
};

} // namespace cpp
} // namespace bulk
