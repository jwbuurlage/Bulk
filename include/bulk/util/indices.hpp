#pragma once

#include <array>

namespace bulk::util {

/** Free functions for flattening multi-indices in volumes. */
template <int D>
int flatten(std::array<int, D> volume, std::array<int, D> idxs) {
    int flattened = 0;
    int offset = 1;
    for (int d = 0; d < D; ++d) {
        flattened += idxs[d] * offset;
        offset *= volume[d];
    }
    return flattened;
}

template <int D>
std::array<int, D> unflatten(std::array<int, D> volume, int flattened) {
    std::array<int, D> unflattened = {};
    for (int d = 0; d < D; ++d) {
        unflattened[d] = flattened % volume[d];
        flattened /= volume[d];
    }
    return unflattened;
}

} // namespace bulk::util
