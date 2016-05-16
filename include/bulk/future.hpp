#pragma once

namespace bulk {

/// Represents a value that will become known in the upcoming superstep
template <typename T, class World>
class future {
  public:
    /// Initializes the future and the buffer
    future(World& world) : world_(world) {
        buffer_ = new T;
    }

    /// Deconstructs the future and its buffer
    ~future() {
        if (buffer_ != nullptr)
            delete buffer_;
    }

    future(future<T, World>& other) = delete;
    void operator=(future<T, World>& other) = delete;

    future(future<T, World>&& other) : world_(other.world_) {
        *this = std::move(other);
    }

    /// Move from one future to another
    void operator=(future<T, World>&& other) {
        auto tmp_buffer = buffer_;
        buffer_ = other.buffer_;
        other.buffer_ = tmp_buffer;
    }

    /// Returns the value held by the local image of the var
    T& value() { return *buffer_; }

    /// Returns the world to which this variable belongs
    World& world() { return world_; }

  private:
    template <typename S, typename Gub>
    friend future<S, Gub> get(int, var<S, Gub>&);

    T* buffer_ = nullptr;
    World& world_;
};

/// Create a future
///
/// \note this function is included so that the programmer does not explicitely
/// has to pass the type of the world
template<typename T, typename World>
typename World::template future<T> create_future(World& world) {
      return future<T, World>(world);
}

} // namespace bulk
