#pragma once

/**
 * \file world.hpp
 *
 * This objects encodes the world of a processor and its place within it.
 */

#include <functional>
#include <memory>
#include <vector>

namespace bulk {

/**
 * This objects encodes the world of a processor and its place within it.
 */
template <class WorldBackend>
class world {
  public:
    using Implementation = typename WorldBackend::implementation;

    template <typename Tag, typename Content>
    using message_container =
        typename WorldBackend::template message_container_type<Tag, Content>;

    template <typename T>
    using var_type = typename WorldBackend::template var_type<T>;

    template <typename T>
    using future_type = typename WorldBackend::template future_type<T>;

    template <typename T>
    using coarray_type = typename WorldBackend::template coarray_type<T>;

    template <typename T>
    using array_type = typename WorldBackend::template array_type<T>;

    /**
     * Retrieve the total number of active processors in a spmd section
     *
     * \returns the number of active processors
     */
    int active_processors() const { return implementation_.active_processors(); }

    /**
     * Retrieve the local processor id
     *
     * \returns an integer containing the id of the local processor
     */
    int processor_id() const { return implementation_.processor_id(); }

    /**
     * Retrieve the id of the next logical processor
     *
     * \returns an integer containing the id of the next processor
     */
    int next_processor() const {
        auto next = processor_id() + 1;
        if (next >= active_processors())
            next -= active_processors();
        return next;
    }

    /**
     * Retrieve the id of the previous logical processor
     *
     * \returns an integer containing the id of the previous processor
     */
    int prev_processor() const {
        auto prev = processor_id() + active_processors() - 1;
        if (prev >= active_processors())
            prev -= active_processors();
        return prev;
    }

    /**
     * Performs a global barrier synchronization of the active processors.
     */
    void sync() { implementation_.sync(); }

    int register_location_(void* location, size_t size) {
        return implementation_.register_location_(location, size);
    }
    void move_location_(int var_id, void* newlocation) {
        implementation_.move_location_(var_id, newlocation);
    }
    void unregister_location_(void* location) {
        implementation_.unregister_location_(location);
    }
    void unregister_location_(int var_id) {
        implementation_.unregister_location_(var_id);
    }

    /**
     * Retrieve the implementation of the world
     *
     * \returns the distributed system implementation
     */
    Implementation& implementation() { return implementation_; }

  private:
    Implementation implementation_;
};

/**
 * Constructs a variable that is registered with `world`.
 *
 * \param T the type of the variable
 *
 * \returns a newly allocated and registered variable
 */
template <typename T, class World>
typename World::template var_type<T> create_var(World& world) {
    return typename World::template var_type<T>(world);
}

/**
 * Constructs a coarray, and registers it with `world`.
 *
 * \param world the distributed layer in which the coarray is defined.
 * \param size the size of the local coarray
 *
 * \returns a newly allocated and registered coarray
 */
template <typename T, typename World>
typename World::template coarray_type<T> create_coarray(World& world,
                                                        int local_size) {
    return typename World::template coarray_type<T>(world, local_size);
}

} // namespace bulk
