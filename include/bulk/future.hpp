#pragma once

#include <iostream>


namespace bulk {

template <typename T, class Hub>
class future {
  public:
    future(Hub& hub) : hub_(hub) {
        buffer_ = new T;
    }

    ~future() {
        if (buffer_ != nullptr)
            delete buffer_;
    }

    future(future<T, Hub>& other) = delete;
    void operator=(future<T, Hub>& other) = delete;

    future(future<T, Hub>&& other) : hub_(other.hub_) {
        *this = std::move(other);
    }

    void operator=(future<T, Hub>&& other) {
        auto tmp_buffer = buffer_;
        buffer_ = other.buffer_;
        other.buffer_ = tmp_buffer;
    }

    T& value() { return *buffer_; }
    Hub& hub() { return hub_; }

  private:
    template <typename S, typename Gub>
    friend future<S, Gub> get(int, var<S, Gub>&, int, int);

    T* buffer_ = nullptr;
    Hub& hub_;
};

template<typename T, typename Hub>
future<T, Hub> create_future(Hub& hub) {
      return future<T, Hub>(hub);
}

} // namespace bulk
