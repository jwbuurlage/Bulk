#pragma once

#include <chrono>

namespace bulk::util {

using namespace std::chrono;

class timer {
    using clock = high_resolution_clock;

  public:
    timer() : begin_time_(clock::now()) { last_time_ = begin_time_; }

    template <typename resolution = std::milli>
    auto get() {
        auto end_time = clock::now();
        last_time_ = end_time;
        return duration<double, resolution>(end_time - begin_time_).count();
    }

    template <typename resolution = std::milli>
    auto get_change() {
        auto end_time = clock::now();
        auto change = duration<double, resolution>(end_time - last_time_).count();
        last_time_ = end_time;
        return change;
    }

  private:
    const time_point<clock> begin_time_;
    time_point<clock> last_time_;
};

} // namespace bulk::util
