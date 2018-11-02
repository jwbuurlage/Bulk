#pragma once

#include <bulk/bulk.hpp>

namespace psc {

template <typename T>
class vector {
  public:
    vector(bulk::world& world, bulk::partitioning<1>& partitioning,
           bulk::coarray<T>&& data)
        : world_(world), partitioning_(partitioning), data_(std::move(data)) {}

    vector(bulk::world& world, bulk::partitioning<1>& partitioning, T value = 0)
        : world_(world), partitioning_(partitioning),
          data_(world_, partitioning.local_count(world_.rank()), value) {}

    T& operator[](int k) { return data_[k]; }
    const T& operator[](int k) const { return data_[k]; }

    vector<T> operator+(const vector<T>& other) {
        psc::vector<T> x(world_, partitioning_);

        for (auto i = 0u; i < data_.size(); ++i) {
            x[i] = data_[i] + other[i];
        }

        return x;
    }

    vector<T> operator-(const vector<T>& other) {
        psc::vector<T> x(world_, partitioning_);

        for (auto i = 0u; i < data_.size(); ++i) {
            x[i] = data_[i] + other[i];
        }

        return x;
    }

    void operator+=(const vector<T>& other) {
        for (auto i = 0u; i < data_.size(); ++i) {
            data_[i] += other[i];
        }
    }

    void operator-=(const vector<T>& other) {
        for (auto i = 0u; i < data_.size(); ++i) {
            data_[i] -= other[i];
        }
    }

    auto size() const { return data_.size(); }
    auto global_size() const { return partitioning_.global_size(); }

    bulk::world& world() const { return world_; }
    bulk::partitioning<1>& partitioning() const { return partitioning_; }

    auto begin() { return data_.begin(); }
    auto end() { return data_.end(); }

  private:
    bulk::world& world_;
    bulk::partitioning<1>& partitioning_;
    bulk::coarray<T> data_;
};

template <typename T>
T dot(const vector<T>& x, const vector<T>& y) {
    bulk::var<T> result(x.world());

    for (auto i = 0u; i < x.size(); ++i) {
        result += x[i] * y[i];
    }

    auto alpha = bulk::foldl(result, [](auto& lhs, auto rhs) { lhs += rhs; });
    return alpha;
}

} // namespace psc
