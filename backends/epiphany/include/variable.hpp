#pragma once
#include "world_state.hpp"

/**
 * \file variable_direct.hpp
 *
 * This header defines a distributed variable, which has a value on each
 * processor. This is a version for systems with a shared memory address space.
 */

namespace bulk {
namespace epiphany {

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
    var(World&) : value_(T()) {
        var_id_ = state.register_location_(&value_, sizeof(T));
    }


    /**
     * Deconstructs and deregisters the variable with the world
     */
    ~var() {
        if (var_id_ != -1)
            state.unregister_location_(var_id_);
    }

    var(var<T, World>& other) = delete;
    void operator=(var<T, World>& other) = delete;

    /**
      * Move constructor: move from one var to a new one
      */
    var(var<T, World>&& other)
        : value_(other.value_), var_id_(other.var_id_) {
        // Note that `other` will be deconstructed right away,
        // and since we take over its `var_id_` we have to make sure
        // that `other` does not unregister it by setting it to -1
        other.var_id_ = -1;
        state.move_location_(var_id_, &value_);
    }

    /**
     * Move assignment: move from one var to an existing one
     */
    void operator=(var<T, World>&& other) {
        if (this != &other) {
            // Note that `other` will be deconstructed right away.
            // Unlike the move constructor above, we already have a `var_id_`
            // so we do NOT take over the `var_id_` of `other`.
            // Therefore we only have to copy its `value_`.
            value_ = other.value_;
        }
    }

    /**
     * Explicitly get the value held by the local image of the var
     *
     * \returns a reference to the value held by the local image
     */
    T& value() { return value_; }

    /**
     * Implicitly get the value held by the local image of the var
     *
     * \note This is for code like `myint = myvar + 5;`.
     */
    operator T&() { return value_; }
    operator const T&() const { return value_; }

    /**
     * Write to the local image
     *
     * \note This is for code like `myvar = 5;`.
     */
    var<T, World>& operator=(const T& rhs) {
        value_ = rhs;
        return *this;
    }

    /**
     * Get a reference to a remove copy of the variable.
     */
    T& operator()(int pid) const {
        // If pid is the local pid, this will not be a `const` function
        // However we want the compiler to optimize so we denote it as `const`
        // so that `for (int i ...) { a(pid)[i]; }` might not call the ()
        // operator every iteration
        return *((T*)state.get_direct_address_(pid, var_id_));
    }

    /**
     * Retrieve the world to which this var is registed.
     *
     * \returns a reference to the world of the var
     */
    World& world() const {
        extern World world;
        return world;
    }

  private:
    T value_;
    int var_id_;
};

} // namespace epiphany
} // namespace bulk
