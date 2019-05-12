#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>

namespace bulk::thread {

// Reference:
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

// Reference:
// https://stackoverflow.com/questions/8115267/writing-a-spinning-thread-barrier-using-c11-atomics
class spinning_barrier {
  public:
    spinning_barrier(unsigned int n) : n_(n), nwait_(0), step_(0) {}

    bool wait() {
        // arrived at the barrier, store the superstep we are in
        unsigned int step = step_.load();

        if (nwait_.fetch_add(1) == n_ - 1) {
            // last thread has arrived at this synchronization point, we reset
            // the counter
            nwait_.store(0);
            step_.fetch_add(1);
            return true;
        } else {
            // check if we are still in the same superstep
            while (step_.load() == step) {
            }
            return false;
        }
    }

  protected:
    const unsigned int n_;
    std::atomic<unsigned int> nwait_;
    std::atomic<unsigned int> step_;
};

} // namespace bulk::thread
