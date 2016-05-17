#pragma once

/**
 * \file coarray.hpp
 *
 * This header provides an implementation of a coarray, which is syntactically
 * similar to the co-arrays defined in Co-array Fortran.
 */

#include "array.hpp"
#include "communication.hpp"


namespace bulk {

/**
 * Distributed array with easy element access, loosely based on the behaviour
 * of Co-Array Fortran.
 *
 * Co-arrays provide a convenient way to share data across processors. Instead
 * of manually sending and receiving data elements, co-arrays model distributed
 * data as a 2-dimensional array, where the first dimension is over the
 * processors, and the second dimension is over local 1-dimensional array
 * indices.
 *
 * It can be used as follows:
 * \code{.cpp}
 *     auto xs = create_coarray<int>(world, 10);
 *     // set the 5th element on the 1st processor to 4
 *     xs(1)[5] = 4;
 *     // set the 3rd element on the local processor to 2
 *     xs[3] = 2;
 * \endcode
 */
template <typename T, class World>
class coarray {
  public:
    class writer {
      public:
        /**
         * \brief Assign a value to a remote image element.
         * 
         * \param value the new value of the element
         */
        void operator=(T value) {
            bulk::put<T>(t_, value, parent_.data(), i_, 1);
        }

      private:
        friend coarray<T, World>;

        writer(World& world, coarray<T, World>& parent, int t, int i)
            : world_(world), parent_(parent), t_(t), i_(i) {}

        World& world_;
        coarray<T, World>& parent_;
        int t_;
        int i_;
    };

    class image {
      public:
        image(World& world, coarray<T, World>& parent, int t)
            : world_(world), parent_(parent), t_(t) {}

        /**
         * \brief Obtain a writer to the remote element.
         *
         * \param i the index of the remote element
         *
         * \returns an object that can be used to write to the remote element
         */
        writer operator[](int i) { return writer(world_, parent_, t_, i); }

      private:
        World& world_;
        coarray<T, World>& parent_;
        int t_;
    };

    /**
     * \brief Initialize and registers the coarray with the world
     *
     * \param world the distributed layer in which the array is defined.
     * \param local_size the size of the local array
     */
    coarray(World& world, int local_size) : world_(world), data_(world_, local_size) {}

    /**
     * \brief Initialize and registers the coarray with the world
     *
     * \param world the distributed layer in which the array is defined.
     * \param local_size the size of the local array
     * \param default_value the initial value of each local element
     */
    coarray(World& world, int local_size, T default_value)
        : world_(world), data_(world_, local_size) {
        for (int i = 0; i < local_size; ++i) {
            data_[i] = default_value;
        }
    }

    /**
     * \brief Retrieve the coarray image with index t
     *
     * \param t index of the target image
     *
     * \returns the coarray image with index t
     */
    image operator()(int t) {
        return image(world_, *this, t);
    }

    /**
     * \brief Access the `i`th element of the local coarray image
     *
     * \param i index of the element
     * \returns reference to the i-th element of the local image
     */
    T& operator[](int i) { return data_[i]; }

    /**
     * \brief Retrieve the world to which this coarray is registed.
     * \returns a reference to the world of the coarray
     */
    World& world() { return world_; }

  private:
    friend image;
    friend writer;

    World& world_;
    array<T, World> data_;

    array<T, World>& data() { return data_; }
};

/**
 * \brief Constructs a coarray, and registers it with `world`.
 *
 * \param world the distributed layer in which the coarray is defined.
 * \param size the size of the local coarray
 * 
 * \returns a newly allocated and registered coarray
 */
template<typename T, typename World>
typename World::template coarray_type<T> create_coarray(World& world, int local_size) {
      return coarray<T, World>(world, local_size);
}

} // namespace bulk
