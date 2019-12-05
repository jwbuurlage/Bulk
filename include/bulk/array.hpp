#pragma once

#include <cstddef>
#include <iostream>
#include <memory>
#include <type_traits>

#include "future.hpp"
#include "variable.hpp"

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
class array : var_base {
    static_assert(std::is_trivially_copyable<T>::value,
                  "(Co)arrays only support trivially-copyable types");

  public:
    /**
     * Construct an array, and register it with `world`.
     *
     * \param world the world in which the array is defined.
     * \param size the local size of the array
     */
    array(bulk::world& world, std::size_t size) : world_(world), size_(size) {
        data_ = new T[size];
        id_ = world_.register_variable_(this);
    }

    /**
     * Construct an array, and register it with `world`.
     *
     * \param world the world in which the array is defined.
     * \param size the local size of the array
     * \param buffer an externally managed data buffer
     */
    array(bulk::world& world, std::size_t size, T* buffer)
    : world_(world), size_(size) {
        data_ = buffer;
        id_ = world_.register_variable_(this);
        external_delete_ = true;
    }

    /**
     * Deconstruct an array, and unregister it with `world`.
     */
    ~array() {
        if (data_) {
            world_.unregister_variable_(id_);
        }

        if (!external_delete_) {
            delete[] data_;
        }
    }

    /**
     * Move an array.
     */
    array(array&& other)
    : world_(other.world_), data_(std::move(other.data_)), id_(other.id_) {
        other.data_ = nullptr;
        other.id_ = -1;
    }

    /**
     * Get an element of the local array.
     *
     * \returns a reference to element \c i in the local array
     */
    T& operator[](size_t i) { return data_[i]; }
    const T& operator[](size_t i) const { return data_[i]; }

    /**
     * Get a reference to the world of the array.
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
     * Put raw values into a remote array image.
     *
     * The cost is `g * count * t`, where `t` is the size of `T`.
     *
     * \param processor the id of a remote processor
     * \param values a pointer to the values
     * \param offset the element where the writing should start
     * \param count the number of elements to be written
     */
    void put(int processor, const T* values, size_t offset, size_t count = 1) {
        world_.put_(processor, values, sizeof(T), id_, offset, count);
    }

    /**
     * Put a range of values to another processor
     */
    template <typename FwdIterator>
    void put(int processor, FwdIterator first, FwdIterator last, size_t offset = 0) {
        std::vector<T> values;
        for (; first != last; ++first) {
            values.push_back(*first);
        }
        world_.put_(processor, values.data(), sizeof(T), id_, offset, values.size());
    }

    /**
     * Get a future to a remote image of an array element.
     *
     * The cost is `g`
     *
     * \param processor the id of the remote processor
     * \param offset the element where the reading should start
     *
     * \returns a `future` to the image.
     */
    future<T> get(int processor, size_t offset) {
        future<T> result(world_);
        world_.get_(processor, id_, sizeof(T), &result.value(), offset, 1);
        return result;
    }

    future<T[]> get(int processor, size_t offset, size_t count) {
        future<T[]> result(world_, count);
        world_.get_(processor, id_, sizeof(T), result.buffer(), offset, count);
        return result;
    }

    // FIXME: These are unusued..., once we use 'backend buffers' for arrays we
    // can redo this
    void deserialize_put(size_t, char*) override final {}
    void serialize(void*) override final {}
    size_t serialized_size() override final { return 0; }
    std::pair<void*, size_t> location_and_size() override final {
        return {data_, size_ * sizeof(T)};
    }

    /**
     * Get the local size of the array
     */
    std::size_t size() const { return size_; }

  private:
    bulk::world& world_;
    T* data_;
    std::size_t size_;
    int id_;
    bool external_delete_ = false;
};

} // namespace bulk
