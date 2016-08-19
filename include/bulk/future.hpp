#pragma once
#include "variable_indirect.hpp"

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
template <typename T, class World>
class future {
  public:
    /**
     * Initializes the future and the buffer
     */
    future(World& world) : world_(world) {
        buffer_ = std::make_unique<T>();
    }

    /**
     * Deconstructs the future and its buffer
     */
    ~future() {}

    future(const future<T, World>& other) = delete;
    void operator=(const future<T, World>& other) = delete;

    future(future<T, World>&& other) : world_(other.world_) {
        *this = std::move(other);
    }

    /**
     * Move a future to another
     */
    void operator=(future<T, World>&& other) {
        buffer_ = std::move(other.buffer_);
    }

    /**
     * Returns the value held by the future
     *
     * \returns a reference to the value
     */
    T& value() { return *buffer_.get(); }

    /**
     * Retrieve the world to which this future is registed.
     * \returns a reference to the world of the future
     */
    World& world() { return world_; }

  private:
    std::unique_ptr<T> buffer_;
    World& world_;
};

} // namespace bulk
