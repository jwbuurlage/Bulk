#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <functional>

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
    explicit barrier(std::size_t iCount)
        : mThreshold(iCount), mCount(iCount), mGeneration(0) {}

    void wait() {
        auto lGen = mGeneration;
        std::unique_lock<std::mutex> lLock{mMutex};
        if (!--mCount) {
            mGeneration++;
            mCount = mThreshold;
            mCond.notify_all();
        } else {
            mCond.wait(lLock, [this, lGen] { return lGen != mGeneration; });
        }
    }

  private:
    std::mutex mMutex;
    std::condition_variable mCond;
    std::size_t mThreshold;
    std::size_t mCount;
    std::size_t mGeneration;
};

// single `world_state` instance shared by every thread
class world_state {
  public:
    explicit world_state(int processors) : sync_barrier(processors) {}

    barrier sync_barrier;
    // Used in var and coarray creation mechanism
    void* var_pointer_;
};

// separate `world_provider` instance for every thread
class world_provider {
  public:
    world_provider() {}
    ~world_provider() {}

    int active_processors() const { return nprocs_; }
    int processor_id() const { return pid_; }

    void barrier() { state_->sync_barrier.wait(); }

    void sync() {
        barrier();
        for (auto& op : sync_operations_)
            op();
        barrier();
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
