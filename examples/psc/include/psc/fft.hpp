#pragma once

#include <complex>

#include <bulk/bulk.hpp>

#include "vector.hpp"

namespace psc {

template <typename T>
vector<std::complex<T>> fft(const vector<T>& x) {
    vector<std::complex<T>> y(x.world(), x.partitioning());
    return y;
}

template <typename T>
vector<T> ifft(const vector<std::complex<T>>& x) {
    vector<T> y(x.world(), x.partitioning());
    return y;
}

} // namespace psc
