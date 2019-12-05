#pragma once

#include "util/meta_helpers.hpp"
#include "util/serialize.hpp"

#include <bulk/world.hpp>
#include <memory>

/**
 * \file future.hpp
 *
 * This header defines a future object, which is used to refer to remote values
 * that are not yet known.
 */

namespace bulk {

class future_base {
  public:
    virtual ~future_base() = default;

    virtual void deserialize_get(size_t size, char* data) = 0;
    virtual int id() const = 0;
};

/**
 * Represents a value that will become known in the upcoming superstep
 */
template <typename T>
class future {
  public:
    using value_type = typename bulk::meta::representation<T>::type;

    /**
     * Initialize the future.
     */
    future(bulk::world& world) { impl_ = std::make_unique<impl>(world); }

    /**
     * Deconstruct the future.
     */
    ~future() {}

    future(const future<T>& other) = delete;
    void operator=(const future<T>& other) = delete;

    /**
     * Move a future.
     */
    future(future<T>&& other) { impl_ = std::move(other.impl_); }
    void operator=(future<T>&& other) { impl_ = std::move(other.impl_); }

    /**
     * Get a reference to the value held by the future.
     *
     * \returns a reference to the value
     */
    value_type& value() { return impl_->buffer_; }
    const value_type& value() const { return impl_->buffer_; }

    /**
     * Implicitly get the value held by the future
     *
     * \note This is for code like `myint = myfuture + 5;`.
     */
    operator value_type&() { return this->value(); }
    operator const value_type&() const { return this->value(); }

    /**
     * Get a reference to the world of the future.
     *
     * \returns a reference to the world of the future
     */
    bulk::world& world() { return impl_->world_; }

  private:
    class impl : public future_base {
      public:
        impl(bulk::world& world) : buffer_{}, world_(world) {
            id_ = world_.register_future_(this);
        }
        ~impl() { world_.unregister_future_(this); }

        // No copies or moves
        impl(impl& other) = delete;
        impl(impl&& other) = delete;
        void operator=(impl& other) = delete;
        void operator=(impl&& other) = delete;

        void deserialize_get(size_t size, char* data) override final {
            auto membuf = bulk::detail::memory_buffer(size, data);
            auto obuf = bulk::detail::omembuf(membuf);
            bulk::detail::fill(obuf, buffer_);
        }

        int id() const override final { return id_; }

        value_type buffer_;
        bulk::world& world_;
        int id_;
    };
    std::unique_ptr<impl> impl_;

    template <typename U>
    friend class var;
    operator future_base*() { return impl_.get(); }
};

template <typename T>
class future<T[]> {
  public:
    /**
     * Initialize the future.
     */
    future(bulk::world& world, int size) : world_(world) {
        buffer_ = std::unique_ptr<T[]>(new T[size]);
    }

    /**
     * Deconstruct the future.
     */
    ~future() {}

    future(const future<T[]>& other) = delete;
    void operator=(const future<T[]>& other) = delete;

    future(future<T[]>&& other) : world_(other.world_) {
        *this = std::move(other);
    }

    void operator=(future<T[]>&& other) { buffer_ = std::move(other.buffer_); }

    /**
     * ...
     */
    T& operator[](int i) { return buffer_[i]; }

    /**
     * Get a reference to the world of the future.
     *
     * \returns a reference to the world of the future
     */
    bulk::world& world() { return world_; }

    T* buffer() { return buffer_.get(); }

  private:
    std::unique_ptr<T[]> buffer_;
    bulk::world& world_;
};

} // namespace bulk
