#pragma once

namespace bulk {

template <typename T, class Hub>
class array {
  public:
    array(Hub& hub, int size) : hub_(hub) {
        data_ = new T[size];
        hub_.register_location_(data_, sizeof(T) * size);
    }

    ~array() {
        if (data_ != nullptr) {
            hub_.unregister_location_(data_);
            delete[] data_;
        }
    }

    T* data() { return data_; }
    T& operator[](int i) { return data_[i]; }

    Hub& hub() { return hub_; }

  private:
    Hub& hub_;
    T* data_;
};

template<typename T, typename Hub>
array<T, Hub> create_array(Hub& hub, int size) {
      return array<T, Hub>(hub, size);
}

} // namespace bulk
