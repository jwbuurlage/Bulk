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
    var(bulk::world& world) : world_(world) {
        value_ = std::make_unique<T>();
        id_ = world_.register_location_(value_.get());
        world_.barrier();
    }

    /**
     * Deconstructs and deregisters the variable with the world
     */
    ~var() {
        // It could be that some core is already unregistering while
        // another core is still reading from the variable. Therefore
        // use a barrier
        world_.barrier();
        if (value_.get())
            world_.unregister_location_(id_);
    }

    // A variable can not be copied
    var(var<T>& other) = delete;
    void operator=(var<T>& other) = delete;

    /**
      * Move from one var to another
      */
    var(var<T>&& other) : world_(other.world_) {
        // Since other no longer has a value_ after a move it will not unregister
        // id_ so we can simply copy it and it will remain valid
        value_ = std::move(other.value_);
        id_ = other.id_;
    }

    /**
     * Move from one var to another
     */
    void operator=(var<T>&& other) {
        if (value_.get())
            world_.unregister_location_(id_);
        // Since other no longer has a value_ after a move it will not unregister
        // id_ so we can simply copy it and it will remain valid
        value_ = std::move(other.value_);
        id_ = other.id_;
    }

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
    operator T&() { return *value_.get(); }
    operator const T&() const { return *value_.get(); }

    /**
     * Write to the local image
     *
     * \note This is for code like `myvar = 5;`.
     */
    var<T>& operator=(const T& rhs) {
        *value_.get() = rhs;
        return *this;
    }

    /**
     * Returns the value held by the local image of the var
     *
     * \returns a reference to the value held by the local image
     */
    T& value() { return *value_.get(); }

    /**
     * Retrieve the world to which this var is registed.
     *
     * \returns a reference to the world of the var
     */
    bulk::world& world() { return world_; }

    // TODO: Make this private and make `image` a friend?
    int id() const { return id_; }
  private:
    std::unique_ptr<T> value_;
    bulk::world& world_;
    int id_;
};

} // namespace bulk
