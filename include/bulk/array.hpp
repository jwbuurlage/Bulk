#pragma once

/**
 * \file array.hpp
 *
 * This header provides a distributed array to be used for communication.
 */

namespace bulk {

/**
 * A distributed variable representing an array on each processor.
 *
 * This object is the default implementation of a distributed _array_.
 * Specialized arrays can be provided by providers, but will always behave like
 * this array.
 */
template <typename T, class World>
class array {
  public:
    /**
     * Constructs an array, and registers it with `world`.
     *
     * \param world the distributed layer in which the array is defined.
     * \param size the size of the local array
     */
    array(World& world, int size) : world_(world) {
        data_ = new T[size];
        world_.register_location_(data_, sizeof(T) * size);
    }

    /**
     * Destructs an array, and unregisters it with `world`.
     */
    ~array() {
        if (data_ != nullptr) {
            world_.unregister_location_(data_);
            delete[] data_;
        }
    }

    /**
     * Retrieve the underlying local data.
     *
     * \returns a pointer to the local data
     */
    T* data() { return data_; }

    /**
     * Retrieve an element of the local array.
     *
     * \returns a reference to element \c i in the local array
     */
    T& operator[](int i) { return data_[i]; }

    /**
     * Retrieve the world to which this array is registed.
     *
     * \returns a reference to the world of the array
     */
    World& world() { return world_; }

  private:
    World& world_;
    T* data_;
};

/**
 * Constructs an array, and registers it with `world`.
 *
 * \param world the distributed layer in which the array is defined.
 * \param size the size of the local array
 *
 * \returns a newly allocated and registered array
 */
template<typename T, typename World>
typename World::template array_type<T> create_array(World& world, int size) {
      return array<T, World>(world, size);
}

} // namespace bulk
