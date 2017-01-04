#pragma once
#include <algorithm>
#include <condition_variable>
#include <cstdio>
#include <functional>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
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

// single `world_state` instance shared by every thread
class world_state {
  public:
    explicit world_state(int processors) : sync_barrier(processors) {}

    barrier sync_barrier;
    // Used in var and coarray creation mechanism
    void* var_pointer_;

    std::mutex log_mutex; // mutex for the vector and for sending output
    std::vector<std::pair<int,std::string>> logs;
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

    int active_processors() const override final { return nprocs_; }
    int processor_id() const override final { return pid_; }

    void barrier() override final { state_->sync_barrier.wait(); }

    void sync() override final {
        barrier();
        // Perform operations required at each sync like
        // swapping message queues
        for (auto& op : sync_operations_)
            op();

        // Print any log messages
        if (pid_ == 0) {
            auto& logs = state_->logs;
            std::stable_sort(logs.begin(), logs.end());
            if (state_->log_callback == nullptr) {
                for (auto& log : logs)
                    std::cout << log.second;
                std::cout << std::flush;
            } else {
                for (auto& log : logs)
                    state_->log_callback(log.first, log.second);
            }
            logs.clear();
        }
        barrier();
    }

    void log_(std::string message) override final {
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
                std::cout << logmessage << std::flush;
            else
                state_->log_callback(pid_, logmessage);
        }
    }

    void abort() override final {
        //TODO
        return;
    }

    // FIXME: Change the whole internal_get_ thing
    template <typename T, class World,
              template <typename, class> class var_type>
    void internal_get_(int processor, var_type<T, World>& the_variable,
                       T& target) {
        target = the_variable.get_ref(processor);
    }

    template <typename T, class World,
              template <typename, class> class var_type>
    void internal_put_(int processor, T value,
                       var_type<T, World>& the_variable) {
        the_variable.get_ref(processor) = value;
    }

    void init_(world_state* state, int pid, int nprocs) {
        state_ = state;
        pid_ = pid;
        nprocs_ = nprocs;
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

    void unregister_sync_operation_(int id) {
        sync_operations_[id] = nullptr;
    }

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
    // TODO
    int register_location_(void* location) override final { return 0; }
    void unregister_location_(int id) override final { return; }

    void put_(int processor, void* value, int size, int var_id) override final {
        return;
    }
    // Size is per element
    void put_(int processor, void* values, int size, int var_id, int offset,
              int count) override final {
        return;
    }
    void get_(int processor, int var_id, int size,
              void* target) override final {
        return;
    }
    // Size is per element
    void get_(int processor, int var_id, int size, void* target, int offset,
              int count) override final {
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
