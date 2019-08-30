#pragma once

/**
 * \file coarray.hpp
 *
 * This header provides an implementation of a coarray, which is syntactically
 * similar to the coarrays defined in Co-array Fortran.
 */

#include <cassert>
#include <cstddef>
#include <vector>

#include "array.hpp"
#include "communication.hpp"
#include "util/span.hpp"

namespace bulk {

/**
 * Distributed array with easy element access, loosely based on the behaviour
 * of Co-Array Fortran.
 *
 * Coarrays provide a convenient way to share data across processors. Instead
 * of manually sending and receiving data elements, coarrays model distributed
 * data as a 2-dimensional array, where the first dimension is over the
 * processors, and the second dimension is over local 1-dimensional array
 * indices.
 *
 * It can be used as follows:
 * \code{.cpp}
 *     auto xs = bulk::coarray<int>(world, 10);
 *     // set the 5th element on the 1st processor to 4
 *     xs(1)[5] = 4;
 *     // set the 3rd element on the local processor to 2
 *     xs[3] = 2;
 * \endcode
 */
template <typename T>
class coarray {
  public:
    struct slice {
        size_t first;
        size_t last;

	slice(size_t a, size_t b) {
            first = a;
            last = b;
        }

        slice(int a, int b) {
            first = (size_t)a;
            last = (size_t)b;
        }
    };

    class slice_writer {
      public:
        /**
         * Assign a value to a slice of a remote image.
         *
         * \param value the new value of each element in the slice
         */
        void operator=(T value) {
            for (auto i = s_.first; i < s_.last; ++i) {
                parent_.put(t_, i, value);
            }
        }

        /**
         * Assign values to a slice of a remote image.
         *
         * \param values the new values of the slice
         *
         * Note: when sizes do not match, the rule is that
         *   array[{2,10}] = {3,2,4};
         * will fill the array as
         *   {x,x,3,2,4,3,2,4,3,2,x,x};
         * That is to say, the values will repeat to fill the slice.
         */
        void operator=(const std::vector<T>& values) {
            parent_.put(t_, s_, values);
        }

        /**
         * Assign values to a slice of a remote image.
         *
         * \param values a span that refers to the elements
         *
         * Note: when sizes do not match, the rule is that
         *   array[{2,10}] = {3,2,4};
         * will fill the array as
         *   {x,x,3,2,4,3,2,4,3,2,x,x};
         * That is to say, the values will repeat to fill the slice.
         */
        void operator=(const bulk::span<T>& values) {
            parent_.put(t_, s_, values);
        }


        future<T[]> get() { return parent_.get(t_, s_.first, s_.last); }

      private:
        friend coarray<T>;

        slice_writer(coarray<T>& parent, int t, slice s)
        : parent_(parent), t_(t), s_(s) {}

        coarray<T>& parent_;
        int t_;
        slice s_;
    };

    class writer {
      public:
        /**
         * Assign a value to a remote image element.
         *
         * \param value the new value of the element
         */
        void operator=(T value) { parent_.put(t_, i_, value); }

        auto get() { return parent_.get(t_, i_); }

      private:
        friend coarray<T>;

        writer(coarray<T>& parent, int t, size_t i)
        : parent_(parent), t_(t), i_(i) {}

        coarray<T>& parent_;
        int t_;
        int i_;
    };

    class image {
      public:
        image(coarray<T>& parent, int t) : parent_(parent), t_(t) {}

        /**
         * Obtain a writer to the remote element.
         *
         * \param i the index of the remote element
         *
         * \returns an object that can be used to write to the remote element
         */
        writer operator[](size_t i) { return writer(parent_, t_, i); }

        slice_writer operator[](slice s) {
            return slice_writer(parent_, t_, s);
        }

      private:
        coarray<T>& parent_;
        int t_;
    };

    /**
     * Initialize and registers the coarray with the world
     *
     * \param world the distributed layer in which the array is defined.
     * \param local_size the size of the local array
     */
    coarray(bulk::world& world, size_t local_size) : data_(world, local_size) {}

    /**
     * Initialize and registers the coarray with the world
     *
     * \param world the distributed layer in which the array is defined.
     * \param local_size the size of the local array
     * \param default_value the initial value of each local element
     */
    coarray(bulk::world& world, size_t local_size, T default_value)
    : data_(world, local_size) {
        for (auto i = 0u; i < local_size; ++i) {
            data_[i] = default_value;
        }
    }

    /**
     * Initialize and registers the coarray with the world
     *
     * \param world the distributed layer in which the array is defined.
     * \param local_size the size of the local array
     * \param buffer the externally managed data buffer
     */
    coarray(bulk::world& world, size_t local_size, T* buffer)
    : data_(world, local_size, buffer) {}

    coarray(coarray&& other) : data_(std::move(other.data_)) {}

    /**
     * Retrieve the coarray image with index t
     *
     * \param t index of the target image
     *
     * \returns the coarray image with index t
     */
    image operator()(int t) { return image(*this, t); }

    /**
     * Access the `i`th element of the local coarray image
     *
     * \param i index of the element
     * \returns reference to the i-th element of the local image
     */
    T& operator[](size_t i) { return data_[i]; }
    const T& operator[](size_t i) const { return data_[i]; }

    /**
     * Get a reference to the world of the coarray.
     *
     * \returns a reference to the world of the coarray
     */
    bulk::world& world() { return data_.world(); }

    /**
     * Get an iterator to the beginning of the local image of the coarray.
     *
     * \returns a pointer to the first element of the local data.
     */
    T* begin() { return data_.begin(); }

    /**
     * Get an iterator to the end of the local image of the coarray.
     *
     * \returns a pointer beyond the last element of the local data.
     */
    T* end() { return data_.end(); }

    /**
     * Put the value `value` into element `idx` on processor `t`.
     */
    void put(int t, size_t idx, T value) { data_.put(t, &value, idx, 1); }

    /**
     * Put a range of data in a remote image.
     */
    void put(int processor, slice s, const std::vector<T>& values) {
        auto count =
        values.size() < (s.last - s.first) ? values.size() : (s.last - s.first);
        data_.put(processor, values.data(), s.first, count);
    }

    /**
     * Put a span of data in a remote image.
     */
    void put(int processor, slice s, const bulk::span<T>& values) {
        auto count =
        values.size() < (s.last - s.first) ? values.size() : (s.last - s.first);
        data_.put(processor, values.data(), s.first, count);
    }

    /**
     * Put a range of data in a remote image.
     */
    template <typename FwdIterator>
    void put(int processor, FwdIterator first, FwdIterator last, size_t offset) {
        data_.put(processor, first, last, offset);
    }

    /**
     * Get a future to the value of element `idx` on processor `t`.
     */
    future<T> get(int t, size_t idx) { return data_.get(t, idx); }

    future<T[]> get(int t, size_t first, size_t last) {
        return data_.get(t, first, last - first);
    }

    /**
     * Get the size of the coarray.
     */
    std::size_t size() const { return data_.size(); }

    /**
     * Get a raw pointer to the local underlying sequential data buffer.
     */
    T* data() { return std::get<0>(data_.location_and_size()); }

    /**
     * Check if the coarray is empty.
     */
    bool empty() const { return size() == 0; }

  private:
    friend image;
    friend writer;

    array<T> data_;
};

} // namespace bulk
