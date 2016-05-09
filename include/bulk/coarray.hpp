#pragma once

#include <bulk/array.hpp>
#include <bulk/communication.hpp>

namespace bulk {

/// Distributed array with easy element access, loosely based on the behaviour
/// of Co-Array Fortran.
///
/// Co-arrays provide a convenient way to share data across processors. Instead
/// of
/// manually sending and receiving data elements, co-arrays model distributed
/// data
/// as a 2-dimensional array, where the first dimension is over the processors,
/// and the second dimension is over local 1-dimensional array indices.
///
/// \example
///     auto xs = create_coarray<int>(hub, 10);
///     // set the 5th element on the 1st processor to 4
///     xs(1)[5] = 4;
///     // set the 3rd element on the local processor to 2
///     xs[3] = 2;
template <typename T, class Hub>
class coarray {
  public:
    class writer {
      public:
        /// Assign a value to a remote image element
        /// 
        /// \param value the new value of the element
        void operator=(T value) {
            bulk::put<T>(t_, value, parent_.data(), i_, 1);
        }

      private:
        friend coarray<T, Hub>;

        writer(Hub& hub, coarray<T, Hub>& parent, int t, int i)
            : hub_(hub), parent_(parent), t_(t), i_(i) {}

        Hub& hub_;
        coarray<T, Hub>& parent_;
        int t_;
        int i_;
    };

    class image {
      public:
        image(Hub& hub, coarray<T, Hub>& parent, int t)
            : hub_(hub), parent_(parent), t_(t) {}

        /// Returns a writer to the remote element
        /// 
        /// \param i the index of the remote element
        writer operator[](int i) {
            return writer(hub_, parent_, t_, i);
        }

      private:
        Hub& hub_;
        coarray<T, Hub>& parent_;
        int t_;
    };

    /// Initialize and registers the coarray with the hub
    coarray(Hub& hub, int local_size) : hub_(hub), data_(hub_, local_size) {}

    /// Initialize and registers the coarray with the hub. In addition, also
    /// sets the elements to a default value
    coarray(Hub& hub, int local_size, T default_value)
        : hub_(hub), data_(hub_, local_size) {
        for (int i = 0; i < local_size; ++i) {
            data_[i] = default_value;
        }
    }

    /// Returns the coarray image with index t
    ///
    /// \param t index of the target image
    /// \returns the coarray image with index t
    image operator()(int t) {
        return image(hub_, *this, t);
    }

    /// Access the i-th element of the local coarray image
    ///
    /// \param i index of the element
    /// \returns reference to the i-th element of the local image
    T& operator[](int i) { return data_[i]; }

    /// Returns the hub the coarray belongs to
    ///
    /// \returns reference to hub corresponding to the coarray
    Hub& hub() { return hub_; }

  private:
    friend image;
    friend writer;

    Hub& hub_;
    array<T, Hub> data_;

    array<T, Hub>& data() { return data_; }
};

template<typename T, typename Hub>
coarray<T, Hub> create_coarray(Hub& hub, int local_size) {
      return coarray<T, Hub>(hub, local_size);
}

} // namespace bulk
