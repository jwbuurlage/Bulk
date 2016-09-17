#pragma once
#include "world_state.hpp"

/**
 * \file coarray.hpp
 *
 * This header provides the Epiphany implementation of a coarray
 */

namespace bulk {
namespace epiphany {

template <typename T, class World>
class coarray {
  public:
    /**
     * Initialize and registers the coarray with the world
     *
     * \param local_size the number of elements in the local array
     */
    coarray(World&, int local_size) {
        size_ = local_size;
        data_ = new T[size_];
        var_id_ = state.register_location_(data_, sizeof(T) * size_);
    }

    /**
     * Initialize and registers the coarray with the world
     *
     * \param local_size the number of elements in the local array
     * \param default_value the initial value of each local element
     */
    coarray(World&, int local_size, const T& default_value) {
        size_ = local_size;
        data_ = new T[size_];
        var_id_ = state.register_location_(data_, sizeof(T) * size_);
        for (int i = 0; i < size_; i++)
            data_[i] = default_value;
    }

    /**
     * Deconstructs and deregisters the variable with the world
     */
    ~coarray() {
        if (var_id_ != VAR_INVALID)
            state.unregister_location_(var_id_);
        if (data_)
            delete[] data_;
    }

    // No copying
    coarray(coarray<T, World>& other) = delete;
    void operator=(coarray<T, World>& other) = delete;

    /**
     * Move constructor: move from one var to a new one
     */
    coarray(coarray<T, World>&& other) {
        // Note that `other` will be deconstructed right away,
        // and since we take over its `var_id_` we have to make sure
        // that `other` does not unregister it by setting it to -1
        // This object itself does not have a var_id_ or data_ yet.
        size_ = other.size_;
        var_id_ = other.var_id_;
        data_ = other.data_;
        other.var_id_ = VAR_INVALID;
        other.data_ = 0;
    }

    /**
     * Move assignment: move from one var to an existing one
     */
    void operator=(coarray<T, World>&& other) {
        if (this != &other) {
            // Note that `other` will be deconstructed right away.
            // Unlike the move constructor above, we already have a `var_id_`
            // and a `data_` pointer.

            // No need to swap this
            size_ = other.size_;

            // Swap
            auto tmpid = var_id_;
            var_id_ = other.var_id_;
            other.var_id_ = tmpid;

            auto tmpdata = data_;
            data_ = other.data_;
            other.data_ = tmpdata;
        }
    }

    operator T*() { return data_; }
    operator const T*() const { return data_; }

    /**
     * Access the `i`th element of the local coarray image
     *
     * \param i index of the element
     * \returns reference to the i-th element of the local image
     */
    T& operator[](int i) { return data_[i]; }
    const T& operator[](int i) const { return data_[i]; }

    T* operator()(int pid) const {
        return (T*)(state.get_direct_address_(pid, var_id_));
    }

    /**
     * Retrieve the world to which this coarray is registed.
     * \returns a reference to the world of the coarray
     */
    World& world() const {
        extern World world;
        return world;
    }

    /**
     * Get an iterator to the beginning of the local image of the co-array.
     *
     * \returns a pointer to the first element of the local data.
     */
    T* begin() { return data_; }
    const T* begin() const { return data_; }

    /**
     * Get an iterator to the end of the local image of the co-array.
     *
     * \returns a pointer beyond the last element of the local data.
     */
    T* end() { return data_ + size_; }
    const T* end() const { return data_ + size_; }

  private:
    T* data_;
    int size_;
    var_id_t var_id_;
};

} // namespace epiphany
} // namespace bulk
