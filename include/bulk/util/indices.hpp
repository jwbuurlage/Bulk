#pragma once

#include <array>

namespace bulk {

// Wrapper around array<int, D> so that we can specialize D = 1
template <int D>
struct index {
    index() = default;

    index(std::initializer_list<int> xs) {
        std::copy(xs.begin(), xs.end(), indices_.begin());
    }

    int& operator[](int i) { return indices_[i]; }
    const int& operator[](int i) const { return indices_[i]; }

    bool operator==(const index& rhs) const { return indices_ == rhs.indices_; }

    int* begin() { return indices_.begin(); }
    int* end() { return indices_.end(); }
    const int* begin() const { return indices_.begin(); }
    const int* end() const { return indices_.end(); }

    std::array<int, D> get() { return indices_; }
    std::array<int, D> indices_ = {0};
};

// Specialize D = 1 to be an int
template <>
struct index<1> {
    index() = default;
    index(int x) : index_(x) {}
    bool operator==(const index& rhs) const { return index_ == rhs.index_; }

    // For generic code using index<1>
    int& operator[](int i) {
        assert(i == 0);
        return index_;
    }
    const int& operator[](int i) const {
        assert(i == 0);
        return index_;
    }

    int* begin() { return &index_;}
    int* end() { return (&index_) + 1;}
    const int* begin() const { return &index_;}
    const int* end() const { return &index_ + 1;}

    int get() { return index_; }

    int index_;
};

template <int D>
using index_type = index<D>;


namespace util {

/** Free functions for flattening multi-indices in volumes. */
template <int D>
int flatten(index<D> volume, index<D> idxs) {
    int flattened = 0;
    int offset = 1;
    for (int d = 0; d < D; ++d) {
        flattened += idxs[d] * offset;
        offset *= volume[d];
    }
    return flattened;
}

template <int D>
index<D> unflatten(index<D> volume, int flattened) {
    index<D> unflattened;
    for (int d = 0; d < D; ++d) {
        unflattened[d] = flattened % volume[d];
        flattened /= volume[d];
    }
    return unflattened;
}

} // namespace util
} // namespace bulk
