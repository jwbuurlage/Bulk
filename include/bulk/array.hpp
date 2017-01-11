#pragma once

#include <cstddef>

#include "future.hpp"

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
 */
template <typename T>
class array {
   public:
    /**
     * Constructs an array, and registers it with `world`.
     *
     * \param world the distributed layer in which the array is defined.
     * \param size the size of the local array
     */
    array(bulk::world& world, std::size_t size) : world_(world), size_(size) {
        data_ = new T[size];
        id_ = world_.register_location_(data_);
    }

    /**
     * Destructs an array, and unregisters it with `world`.
     */
    ~array() {
        if (data_ != nullptr) {
            world_.unregister_location_(id_);
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
    bulk::world& world() { return world_; }

    /**
     * Get an iterator to the beginning of the local image of the array.
     *
     * \returns a pointer to the first element of the local data.
     */
    T* begin() { return data_; }

    /**
     * Get an iterator to the end of the local image of the array.
     *
     * \returns a pointer beyond the last element of the local data.
     */
    T* end() { return data_ + size_; }

    /**
     * Put values to another processor
     *
     */
    void put(int processor, T* values, int offset, int count = 1) {
        world_.put_(processor, values, sizeof(T), id_, offset, count);
    }

    future<T> get(int processor, int offset, int count = 1) {
        // TODO future with count other than 1
        future<T> result(world_);
        world_.get_(processor, id_, sizeof(T), &result.value(), offset, 1);
        return result;
    }

    std::size_t size() const {
        return size_;
    }

   private:
    bulk::world& world_;
    T* data_;
    std::size_t size_;
    int id_;
};

}  // namespace bulk
