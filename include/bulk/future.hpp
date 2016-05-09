#pragma once

namespace bulk {

/// Represents a value that will become known in the upcoming superstep
template <typename T, class Hub>
class future {
  public:
    /// Initializes the future and the buffer
    future(Hub& hub) : hub_(hub) {
        buffer_ = new T;
    }

    /// Deconstructs the future and its buffer
    ~future() {
        if (buffer_ != nullptr)
            delete buffer_;
    }

    future(future<T, Hub>& other) = delete;
    void operator=(future<T, Hub>& other) = delete;

    future(future<T, Hub>&& other) : hub_(other.hub_) {
        *this = std::move(other);
    }

    /// Move from one future to another
    void operator=(future<T, Hub>&& other) {
        auto tmp_buffer = buffer_;
        buffer_ = other.buffer_;
        other.buffer_ = tmp_buffer;
    }

    /// Returns the value held by the local image of the var
    T& value() { return *buffer_; }

    /// Returns the hub to which this variable belongs
    Hub& hub() { return hub_; }

  private:
    template <typename S, typename Gub>
    friend future<S, Gub> get(int, var<S, Gub>&);

    T* buffer_ = nullptr;
    Hub& hub_;
};

/// Create a future
///
/// \note this function is included so that the programmer does not explicitely
/// has to pass the type of the hub
template<typename T, typename Hub>
future<T, Hub> create_future(Hub& hub) {
      return future<T, Hub>(hub);
}

} // namespace bulk
