#pragma once

#include <cstddef>

namespace bulk {

/**
 * A simplified `span` object. Will be replaced by `std::span` as soon as we move to C++20.
 */
template <typename T>
class span {
  public:
    span(T* data, std::size_t size) : data_(data), size_(size) {}

    const T* data() const { return data_; }
    std::size_t size() const { return size_; }

  private:
    T* data_;
    std::size_t size_;
};

} // namespace bulk
