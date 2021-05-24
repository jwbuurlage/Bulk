#pragma once

#include <array>
#include <cassert>

namespace bulk {

// Wrapper around array<int, D> so that we can specialize D = 1
template <int D>
struct index {
  index() = default;

  index(std::initializer_list<size_t> xs) {
    std::copy(xs.begin(), xs.end(), indices_.begin());
  }

  size_t& operator[](int i) { return indices_[i]; }
  const size_t& operator[](int i) const { return indices_[i]; }

  bool operator==(const index& rhs) const { return indices_ == rhs.indices_; }

  size_t* begin() { return indices_.begin(); }
  size_t* end() { return indices_.end(); }
  const size_t* begin() const { return indices_.begin(); }
  const size_t* end() const { return indices_.end(); }

  std::array<size_t, D> get() { return indices_; }
  std::array<size_t, D> indices_ = {0};
};

// Specialize D = 1 to be 'size_t'
template <>
struct index<1> {
  index() = default;
  index(size_t x) : index_(x) {}
  bool operator==(const index& rhs) const { return index_ == rhs.index_; }

  // For generic code using index<1>
  size_t& operator[](int i) {
    assert(i == 0);
    return index_;
  }
  const size_t& operator[](int i) const {
    assert(i == 0);
    return index_;
  }

  size_t* begin() { return &index_; }
  size_t* end() { return (&index_) + 1; }
  const size_t* begin() const { return &index_; }
  const size_t* end() const { return &index_ + 1; }

  size_t get() { return index_; }

  size_t index_;
};

template <int D>
using index_type = index<D>;

namespace util {

/** Free functions for flattening multi-indices in volumes. */
template <int D>
size_t flatten(index<D> volume, index<D> idxs) {
  size_t flattened = 0;
  size_t offset = 1;
  for (int d = 0; d < D; ++d) {
    flattened += idxs[d] * offset;
    offset *= volume[d];
  }
  return flattened;
}

template <int D>
index<D> unflatten(index<D> volume, size_t flattened) {
  index<D> unflattened;
  for (int d = 0; d < D; ++d) {
    unflattened[d] = flattened % volume[d];
    flattened /= volume[d];
  }
  return unflattened;
}

}  // namespace util
}  // namespace bulk
