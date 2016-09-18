#pragma once
#include "variable.hpp"

/**
 * \file coarray.hpp
 *
 * This header provides the cpp implementation of a coarray
 */

namespace bulk {
namespace cpp {

template <typename T, class World>
class coarray {
  public:
    /**
     * Initialize and registers the coarray with the world
     *
     * \param local_size the number of elements in the local array
     */
    coarray(World& w, int local_size) : size_(local_size), world_(w) {
        // Below we assume we can use var(1) so have a separate case
        // for single threaded versions
        if (world_.active_processors() == 1) {
            all_values_ = new T[local_size];
            other_values_ = new T*[1];
            other_values_[0] = all_values_;
            self_value_ = all_values_;
        } else {
            auto pid = world_.processor_id();

            // First gather the sizes of all cores in order to know
            // how much core zero should allocate
            var<int, World> x(world_);
            x = local_size;

            // Creating this var includes a barrier so we can assume
            // that x is fine now
            var<T*, World> buffer(world_);

            // Let core 0 do the allocations and also fill the buffer
            if (pid == 0) {
                // Compute total size
                int total_size = 0;
                for (int i = 0; i < world_.active_processors(); i++)
                    total_size += x.get_ref(i);

                all_values_ = new T[total_size];
                other_values_ = new T*[world_.active_processors()];

                // Set the proper pointers
                int total = 0;
                for (int i = 0; i < world_.active_processors(); i++) {
                    other_values_[i] = &all_values_[total];
                    total += x.get_ref(i);
                }

                // Give the buffers to other cores
                buffer(0) = all_values_;
                buffer(1) = (T*)other_values_; // sorry c++ i did not mean to
            }
            world_.sync();
            // All cores save the pointer
            all_values_ = buffer.get_ref(0);
            other_values_ = (T**)buffer.get_ref(1);
            self_value_ = other_values_[pid];
        }
    }

    /**
     * Deconstructs and deregisters the variable with the world
     */
    ~coarray() {
        // It could be that core zero is already deconstructing while
        // another core is still reading from the variable. Therefore
        // use a barrier
        world_.implementation().barrier();
        if (world_.processor_id() == 0) {
            if (all_values_)
                delete[] all_values_;
            if (other_values_)
                delete[] other_values_;
        }
    }

    // No copying
    coarray(coarray<T, World>& other) = delete;
    void operator=(coarray<T, World>& other) = delete;

    /**
     * Move constructor: move from one var to a new one
     */
    coarray(coarray<T, World>&& other)
        : self_value_(other.self_value_), all_values_(other.all_values_),
          other_values_(other.other_values_), size_(other.size_), world_(other.world_) {
        // Note that `other` will be deconstructed right away,
        // and since we take over its buffers we have to make sure
        // that `other` does not deallocate the buffers.
        // This object itself does not have a buffer yet.
        other.all_values_ = 0;
        other.other_values_ = 0;
    }

    /**
     * Move assignment: move from one var to an existing one
     */
    void operator=(coarray<T, World>&& other) {
        if (this != &other) {
            // Note that `other` will be deconstructed right away.
            // Unlike the move constructor above, we already have an allocated
            // buffer. To avoid a memcpy, we swap pointers so that the other
            // object deallocates our old buffers.

            // No need to swap this
            size_ = other.size_;
            std::swap(all_values_, other.all_values_);
            std::swap(other_values_, other.other_values_);
            std::swap(self_value_, other.self_value_);
        }
    }

    operator T*() { return self_value_; }
    operator const T*() const { return self_value_; }

    /**
     * Access the `i`th element of the local coarray image
     *
     * \param i index of the element
     * \returns reference to the i-th element of the local image
     */
    T& operator[](int i) { return self_value_[i]; }
    const T& operator[](int i) const { return self_value_[i]; }

    T* operator()(int pid) const {
        return other_values_[pid];
    }

    /**
     * Retrieve the world to which this coarray is registed.
     * \returns a reference to the world of the coarray
     */
    World& world() const {
        return world_;
    }

    /**
     * Get an iterator to the beginning of the local image of the co-array.
     *
     * \returns a pointer to the first element of the local data.
     */
    T* begin() { return self_value_; }
    const T* begin() const { return self_value_; }

    /**
     * Get an iterator to the end of the local image of the co-array.
     *
     * \returns a pointer beyond the last element of the local data.
     */
    T* end() { return self_value_ + size_; }
    const T* end() const { return self_value_ + size_; }

  private:
    T* self_value_;    // points to local part of the coarray. equal to
                       // other_values_[mypid]
    T* all_values_;    // the complete coarray, one block of memory
    T** other_values_; // other_values_[pid] points to their part of all_values_
    int size_; // local size
    World& world_;
};

} // namespace cpp
} // namespace bulk
