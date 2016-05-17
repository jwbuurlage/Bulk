#pragma once

/**
 * \file future.hpp
 *
 * This header defines a future object, which is used to refer to remote values
 * that are not yet known.
 */

namespace bulk {

/**
 * \brief Represents a value that will become known in the upcoming superstep
 */
template <typename T, class World>
class future {
  public:
    /**
     * \brief Initializes the future and the buffer
     */
    future(World& world) : world_(world) {
        buffer_ = new T;
    }

    /**
     * \brief Deconstructs the future and its buffer
     */
    ~future() {
        if (buffer_ != nullptr)
            delete buffer_;
    }

    future(const future<T, World>& other) = delete;
    void operator=(const future<T, World>& other) = delete;

    future(future<T, World>&& other) : world_(other.world_) {
        *this = std::move(other);
    }

    /**
     * \brief Move a future to another
     */
    void operator=(future<T, World>&& other) {
        auto tmp_buffer = buffer_;
        buffer_ = other.buffer_;
        other.buffer_ = tmp_buffer;
    }

    /**
     * \brief Returns the value held by the future
     *
     * \returns a reference to the value
     */
    T& value() { return *buffer_; }

    /**
     * \brief Retrieve the world to which this array is registed.
     * \returns a reference to the world of the array
     */
    World& world() { return world_; }

  private:
    template <typename S, typename Gub>
    friend future<S, Gub> get(int, var<S, Gub>&);

    T* buffer_ = nullptr;
    World& world_;
};

/**
 * \brief Constructs a future, and registers it with `world`.
 *
 * \param world the distributed layer in which the future is defined.
 * \param size the size of the local future
 * 
 * \returns a newly allocated and registered future
 */
template<typename T, typename World>
typename World::template future<T> create_future(World& world) {
      return future<T, World>(world);
}

} // namespace bulk
