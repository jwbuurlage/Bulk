#pragma once

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
        world_.register_location_(&value_, sizeof(T));
    }

    /**
     * Deconstructs and deregisters the variable with the world
     */
    ~var() { world_.unregister_location_(&value_); }

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
        world_.register_location_(&value_, sizeof(T));
        value_ = other.value();
    }

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
     * Returns the value held by the local image of the var
     *
     * \returns a reference to the value held by the local image
     */
    T& value() { return value_; }


    /**
     * Retrieve the world to which this array is registed.
     *
     * \returns a reference to the world of the array
     */
    World& world() { return world_; }

  private:
    T value_;
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
template<typename T, typename World>
typename World::template var_type<T> create_var(World& world) {
      return var<T, World>(world);
}

} // namespace bulk
