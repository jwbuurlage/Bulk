#pragma once

#include <condition_variable>
#include <mutex>

namespace bulk::thread {

// Taken from
// http://stackoverflow.com/questions/24465533/implementing-boostbarrier-in-c11
// We could instead replace it by the pthread barrier
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

} // namespace bulk::thread
