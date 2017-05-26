#pragma once

#include <chrono>

namespace bulk::util {

using namespace std::chrono;

class timer {
    using clock = high_resolution_clock;

  public:
    timer() : begin_time_(clock::now()) {}

    template <typename resolution = std::milli>
    auto get() {
        auto end_time = clock::now();
        return duration<double, resolution>(end_time - begin_time_).count();
    }

  private:
    const time_point<clock> begin_time_;
};

} // namespace bulk::util
