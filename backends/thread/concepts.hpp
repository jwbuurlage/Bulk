#pragma once

namespace bulk {

template <typename B>
concept Barrier = requires(B b, int processors) {
  B(processors);
  b.arrive_and_wait();
};

}  // namespace bulk
