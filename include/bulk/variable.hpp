#pragma once

#include <iostream>
#include <memory>

/**
 * \file variable.hpp
 *
 * This header defines a distributed variable, which has a value on each
 * processor.
 */

namespace bulk {

template <typename T, class World>
class future;

/**
 * Represents a distributed object with an image for each processor, that is
 * readable and writable from remote processors.
 */
template <typename T, class World>
class var {
  public:
    /**
     * Initialize and registers the variable with the world
     */
    var(World& world) : world_(world) {
        value_ = std::make_unique<T>();
        world_.register_location_(value_.get(), sizeof(T));
    }

    /**
     * Deconstructs and deregisters the variable with the world
     */
    ~var() {
        if (value_.get())
            world_.unregister_location_(value_.get());
    }

    var(var<T, World>& other) = delete;
    void operator=(var<T, World>& other) = delete;

    /**
      * Move from one var to another
      */
    var(var<T, World>&& other) : world_(other.world_) {
        *this = std::move(other);
    }

    /**
     * Move from one var to another
     */
    void operator=(var<T, World>&& other) {
        if (value_.get()) {
            world_.unregister_location_(value_.get());
        }
        value_ = std::move(other.value_);
    }

    /**
     * Implicitly get the value held by the local image of the var
     *
     * \note This is for code like `myint = myvar + 5;`.
     */
    operator T&() { return *value_.get(); }
    operator const T&() const { return *value_.get(); }

    /**
     * Write to the local image
     *
     * \note This is for code like `myvar = 5;`.
     */
    var<T, World>& operator=(const T& rhs) {
        *value_.get() = rhs;
        return *this;
    }

    /**
     * Returns the value held by the local image of the var
     *
     * \returns a reference to the value held by the local image
     */
    T& value() { return *value_.get(); }

    /**
     * Retrieve the world to which this var is registed.
     *
     * \returns a reference to the world of the var
     */
    World& world() { return world_; }

  private:
    std::unique_ptr<T> value_;
    World& world_;
};

/**
 * Constructs a variable, and registers it with `world`.
 *
 * \param world the distributed layer in which the variable is defined.
 * \param size the size of the local variable
 *
 * \returns a newly allocated and registered variable
 */
template <typename T, typename World>
typename World::template var_type<T> create_var(World& world) {
    return typename World::template var_type<T>(world);
}

} // namespace bulk
