#pragma once
#include <utility>

/**
 * \file variable_direct.hpp
 *
 * This header defines a distributed variable, which has a value on each
 * processor. This is a version for systems with a shared memory address space.
 */

namespace bulk {
namespace cpp {

/**
 * Represents a distributed object with an image for each processor, that is
 * readable and writable from remote processors.
 */
template <typename T, class World>
class var {
  public:
    /**
     * Initialize and registers the variable with the world
     */
    var(World& w) : world_(w) {
        auto pid = world_.processor_id();
        if (pid == 0) {
            all_values_ = new T[world_.active_processors()];
            world_.implementation().set_pointer_(all_values_);
        }
        world_.implementation().barrier();
        all_values_ = world_.implementation().template get_pointer_<T>();
        world_.implementation().barrier();
        self_value_ = &all_values_[pid];
    }

    /**
     * Deconstructs and deregisters the variable with the world
     */
    ~var() {
        if (world_.processor_id() == 0 && all_values_ != 0)
            delete[] all_values_;
    }

    // A variable can not be copied
    var(var<T, World>& other) = delete;
    void operator=(var<T, World>& other) = delete;

    /**
      * Move constructor: move from one var to a new one
      */
    var(var<T, World>&& other)
        : self_value_(other.self_value_), all_values_(other.all_values_),
          world_(other.world_) {
        // Note that `other` will be deconstructed right away, so we
        // must make sure that it does not deallocate
        other.all_values_ = 0;
    }

    /**
     * Move assignment: move from one var to an existing one
     */
    void operator=(var<T, World>&& other) {
        if (this != &other) {
            // Note that `other` will be deconstructed right away.
            // Unlike the move constructor above, we already have something
            // allocated ourselves.
            // One of the two should be deallocated. To avoid a memcpy
            // we swap the buffers and the other var will deallocate our
            // old buffer.
            std::swap(self_value_, other.self_value_);
            std::swap(all_values_, other.all_values_);
        }
    }

    /**
     * Explicitly get the value held by the local image of the var
     *
     * \returns a reference to the value held by the local image
     */
    T& value() { return *self_value_; }

    /**
     * Implicitly get the value held by the local image of the var
     *
     * \note This is for code like `myint = myvar + 5;`.
     */
    operator T&() { return *self_value_; }
    operator const T&() const { return *self_value_; }

    /**
     * Write to the local image
     *
     * \note This is for code like `myvar = 5;`.
     */
    var<T, World>& operator=(const T& rhs) {
        *self_value_ = rhs;
        return *this;
    }

    /**
     * Get a reference to a remove copy of the variable.
     */
    T& operator()(int pid) const {
        return all_values_[pid];
    }

    /**
     * Retrieve the world to which this var is registed.
     *
     * \returns a reference to the world of the var
     */
    World& world() const {
        return world_;
    }

  private:
    T* self_value_; // points to all_values_[pid]
    T* all_values_;
    World& world_;
};

} // namespace cpp
} // namespace bulk
