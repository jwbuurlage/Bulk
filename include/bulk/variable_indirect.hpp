#pragma once

#include <bulk/world.hpp>
#include <memory>

/**
 * \file variable_indirect.hpp
 *
 * This header defines a distributed variable, which has a value on each
 * processor. This is a version for systems with no shared memory address space.
 */

namespace bulk {

template <typename T, class World>
class future;

/**
 * Represents a distributed object with an image for each processor, that is
 * readable and writable from remote processors.
 */
template <typename T, class World>
class var_indirect {
  public:
    class image {
      public:
        using var_type = var_indirect<T, World>;
        image(var_type& var, int t) : var_(var), t_(t) {}

        /**
         * Assign a value to a remote image
         *
         * \param value the new value of the image
         */
        void operator=(T value) { bulk::put(t_, value, var_); }

        /**
         * Obtain a future to the remote image value.
         */
        auto get() { return bulk::get(t_, var_); }

      private:
        var_type& var_;
        int t_;
    };

    using value_type = T;

    /**
     * Initialize and registers the variable with the world
     */
    var_indirect(World& world) : world_(world) {
        value_ = std::make_unique<T>();
        world_.implementation().register_location_(value_.get(), sizeof(T));
    }

    /**
     * Deconstructs and deregisters the variable with the world
     */
    ~var_indirect() {
        if (value_.get())
            world_.implementation().unregister_location_(value_.get());
    }

    var_indirect(var_indirect<T, World>& other) = delete;
    void operator=(var_indirect<T, World>& other) = delete;

    /**
      * Move from one var to another
      */
    var_indirect(var_indirect<T, World>&& other) : world_(other.world_) {
        *this = std::move(other);
    }

    /**
     * Obtain a image object to a remote image, added for syntactic sugar
     *
     * \returns a `var_indirect::image` object to the image with index `t`.
     */
    image operator()(int t) { return image(*this, t); };

    /**
     * Move from one var to another
     */
    void operator=(var_indirect<T, World>&& other) {
        if (value_.get()) {
            world_.implementation().unregister_location_(value_.get());
        }
        value_ = std::move(other.value_);
    }

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
    var_indirect<T, World>& operator=(const T& rhs) {
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
    World& world() { return world_; }

  private:
    std::unique_ptr<T> value_;
    World& world_;
};

} // namespace bulk
