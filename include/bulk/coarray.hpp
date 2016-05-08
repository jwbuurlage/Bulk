#pragma once

#include <bulk/array.hpp>
#include <bulk/communication.hpp>

namespace bulk {

template <typename T, class Hub>
class coarray;

template <typename T, class Hub>
class coarray_writer {
  public:
    coarray_writer(Hub& hub, coarray<T, Hub>& parent, int t, int i)
        : hub_(hub), parent_(parent), t_(t), i_(i) {}

    void operator=(T value) { bulk::put<T>(t_, value, parent_.data(), i_, 1); }

  private:
    Hub& hub_;
    coarray<T, Hub>& parent_;
    int t_;
    int i_;
};

template <typename T, class Hub>
class coarray_image {
  public:
    coarray_image(Hub& hub, coarray<T, Hub>& parent, int t)
        : hub_(hub), parent_(parent), t_(t) {}

    coarray_writer<T, Hub> operator[](int i) {
        return coarray_writer<T, Hub>(hub_, parent_, t_, i);
    }

  private:
    Hub& hub_;
    coarray<T, Hub>& parent_;
    int t_;
};

template <typename T, class Hub>
class coarray {
  public:
    coarray(Hub& hub, int local_size) : hub_(hub), data_(hub_, local_size) {}

    coarray_image<T, Hub> operator()(int t) {
        return coarray_image<T, Hub>(hub_, *this, t);
    }

    T& operator[](int i) { return data_[i]; }

  private:
    friend coarray_image<T, Hub>;
    friend coarray_writer<T, Hub>;

    Hub& hub_;
    array<T, Hub> data_;

    array<T, Hub>& data() { return data_; }
};

template<typename T, typename Hub>
coarray<T, Hub> create_coarray(Hub& hub, int local_size) {
      return coarray<T, Hub>(hub, local_size);
}

} // namespace bulk
