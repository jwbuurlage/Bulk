#pragma once

#include "util/meta_helpers.hpp"

#include <bulk/world.hpp>
#include <memory>

/**
 * \file future.hpp
 *
 * This header defines a future object, which is used to refer to remote values
 * that are not yet known.
 */

namespace bulk {

/**
 * Represents a value that will become known in the upcoming superstep
 */
template <typename T>
class future {
    using value_type = typename bulk::meta::representation<T>::type;

  public:
    /**
     * Initialize the future.
     */
    future(bulk::world& world) : world_(world) {
        buffer_ = std::make_unique<T>();
    }

    /**
     * Deconstruct the future.
     */
    ~future() {}

    future(const future<T>& other) = delete;
    void operator=(const future<T>& other) = delete;

    future(future<T>&& other) : world_(other.world_) {
        *this = std::move(other);
    }

    /**
     * Move a future.
     */
    void operator=(future<T>&& other) { buffer_ = std::move(other.buffer_); }

    /**
     * Get a reference to the value held by the future.
     *
     * \returns a reference to the value
     */
    value_type& value() { return *buffer_.get(); }
    const value_type& value() const { return *buffer_.get(); }

    /**
     * Implicitly get the value held by the future
     *
     * \note This is for code like `myint = myfuture + 5;`.
     */
    operator value_type&() { return this->value(); }
    operator const value_type&() const { return this->value(); }

    /**
     * Get a reference to the world of the future.
     *
     * \returns a reference to the world of the future
     */
    bulk::world& world() { return world_; }

  private:
    std::unique_ptr<value_type> buffer_;
    bulk::world& world_;
};

template <typename T>
class future<T[]> {
  public:
    /**
     * Initialize the future.
     */
    future(bulk::world& world, int size) : world_(world) {
        buffer_ = std::unique_ptr<T[]>(new T[size]);
    }

    /**
     * Deconstruct the future.
     */
    ~future() {}

    future(const future<T[]>& other) = delete;
    void operator=(const future<T[]>& other) = delete;

    future(future<T[]>&& other) : world_(other.world_) {
        *this = std::move(other);
    }

    void operator=(future<T[]>&& other) { buffer_ = std::move(other.buffer_); }

    /**
     * ...
     */
    T& operator[](int i) { return buffer_[i]; }

    /**
     * Get a reference to the world of the future.
     *
     * \returns a reference to the world of the future
     */
    bulk::world& world() { return world_; }

    T* buffer() { return buffer_.get(); }

  private:
    std::unique_ptr<T[]> buffer_;
    bulk::world& world_;
};

} // namespace bulk
