#pragma once

#include <bulk/world.hpp>
#include <bulk/future.hpp>
#include <memory>

/**
 * \file variable_indirect.hpp
 *
 * This header defines a distributed variable, which has a value on each
 * processor. This is a version for systems with no shared memory address space.
 */

namespace bulk {

template <typename T>
class future;

/**
 * Represents a distributed object with an image for each processor, that is
 * readable and writable from remote processors.
 */
template <typename T>
class var {
  public:
    class image {
      public:
        image(var<T>& v, int t) : var_(v), t_(t) {}

        /**
         * Assign a value to a remote image
         *
         * \param value the new value of the image
         */
        void operator=(T value) {
            var_.world().put_(t_, &value, sizeof(T), var_.id());
        }

        /**
         * Obtain a future to the remote image value.
         */
        future<T> get() {
            future<T> result(var_.world());
            var_.world().get_(t_, var_.id(), sizeof(T), result.value());
            return result;
        }

      private:
        var<T>& var_;
        int t_;
    };

    using value_type = T;

    /**
     * Initialize and registers the variable with the world
     */
    var(bulk::world& world) {
        impl_ = std::make_unique<var_impl>(world);
        world.barrier();
    }

    /**
     * Deconstructs and deregisters the variable with the world
     */
    ~var() {
        // It could be that some core is already unregistering while
        // another core is still reading from the variable. Therefore
        // use a barrier before var_impl unregisters
        impl_->world.barrier();
    }

    // A variable can not be copied
    var(var<T>& other) = delete;
    void operator=(var<T>& other) = delete;

    /**
      * Move from one var to another
      */
    var(var<T>&& other) { impl_ = std::move(other.impl_); }

    /**
     * Move from one var to another
     */
    void operator=(var<T>&& other) { impl_ = std::move(other.impl_); }

    /**
     * Obtain a image object to a remote image, added for syntactic sugar
     *
     * \returns a `var::image` object to the image with index `t`.
     */
    image operator()(int t) { return image(*this, t); };


    /**
     * Implicitly get the value held by the local image of the var
     *
     * \note This is for code like `myint = myvar + 5;`.
     */
    operator T&() { return impl_->value; }
    operator const T&() const { return impl_->value; }

    /**
     * Write to the local image
     *
     * \note This is for code like `myvar = 5;`.
     */
    var<T>& operator=(const T& rhs) {
        impl_->value = rhs;
        return *this;
    }

    /**
     * Returns the value held by the local image of the var
     *
     * \returns a reference to the value held by the local image
     */
    T& value() { return impl_->value; }

    /**
     * Retrieve the world to which this var is registed.
     *
     * \returns a reference to the world of the var
     */
    bulk::world& world() { return impl_->world; }

    // TODO: Make this private and make `image` a friend?
    int id() const { return impl_->id; }
  private:
    // Default implementation is a value, world and id.
    // Backends can subclass bulk::var<T>::var_impl to add more
    class var_impl {
      public:
        var_impl(bulk::world& world_) : world(world_) {
            id = world.register_location_(&value);
        }
        virtual ~var_impl() { world.unregister_location_(id); }

        // No copies or moves
        var_impl(var_impl& other) = delete;
        var_impl(var_impl&& other) = delete;
        void operator=(var_impl& other) = delete;
        void operator=(var_impl&& other) = delete;

        T value;
        bulk::world& world;
        int id;
    };
    std::unique_ptr<var_impl> impl_;
};

} // namespace bulk
