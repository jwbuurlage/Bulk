#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>

// In the cpp version, the `world` instance is shared amongst all threads
// Therefore anything that alters the internal state of `world_provider`
// should use proper mutex mechanisms

namespace bulk {
namespace cpp {

// Taken from
// http://stackoverflow.com/questions/24465533/implementing-boostbarrier-in-c11
class Barrier {
  public:
    //explicit Barrier(std::size_t iCount)
    //    : mThreshold(iCount), mCount(iCount), mGeneration(0) {}

    void init(std::size_t iCount) {
        mThreshold = iCount;
        mCount = iCount;
        mGeneration = 0;
    }

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

class world_provider {
  public:
    // We would like to initialize barriers and so on here,
    // but because we need `nprocs` it can only be done later
    // since we can not pass arguments in this constructor
    world_provider() {}
    ~world_provider() {}

    int active_processors() const { return nprocs_; }
    int processor_id() const { return pid_; }

    void barrier() {
        sync_barrier.wait();
    }

    void sync() {
        barrier();
    }

    // This should be in the constructor but `world` does not have one
    // so its done this way. It should only be called ONCE,
    // so NOT for every thread separately.
    // environment will:
    // - construct `world`
    // - call init_(nprocs) 
    // - spawn the threads
    // - something to determine pid??
    void init_(int nprocs) {
        nprocs_ = nprocs;
        sync_barrier.init(nprocs);
    }

  private:
    int pid_ = 0;
    int nprocs_ = 0;
    
    Barrier sync_barrier;
};

} // namespace cpp
} // namespace bulk
