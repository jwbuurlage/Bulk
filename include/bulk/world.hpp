#pragma once

/**
 * \file world.hpp
 *
 * This objects encodes the world of a processor and its place within it.
 */

#include "messages.hpp"

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
    using queue_type = typename WorldBackend::template queue_type<Tag, Content>;

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
    int active_processors() const {
        return implementation_.active_processors();
    }

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
     * Performs a global barrier synchronization of the active processors
     * and resolves any outstanding communication. Messages previously
     * received in queues are cleared for the next superstep.
     * The function must be called by all processors.
     * When some processors call `sync` while others call `barrier`
     * at the same time, behaviour is undefined.
     */
    void sync() { implementation_.sync(); }

    /**
     * Performs a global barrier synchronization of the active processors
     * without resolving outstanding communication. Queues are not cleared.
     * The function must be called by all processors.
     * When some processors call `sync` while others call `barrier`
     * at the same time, behaviour is undefined.
     */
    void barrier() { implementation_.barrier(); }

    /**
     * Print output for debugging, shown at the next synchronization.
     *
     * The syntax is printf style.
     *
     * The logs are sorted by processor id before being printed at the next
     * call to `sync`.
     *
     * At the next call to `sync`, the logs of all processors are sent
     * to the `environment` and by default printed to standard output.
     * Instead one can use `environment::set_log_callback` to intercept
     * these log messages.
     */
    template <typename... Ts>
    void log(const char* format, const Ts&... ts) {
        implementation_.log(format, ts...);
    }

    /**
     * Print output for debugging, guaranteed to be shown before the function
     * returns.
     *
     * The syntax is printf style.
     *
     * The log message is sent to the `environment` and by default
     * printed to standard output.
     * Instead one can use `environment::set_log_callback` to intercept
     * these log messages.
     */
    template <typename... Ts>
    void log_direct(const char* format, const Ts&... ts) {
        implementation_.log_direct(format, ts...);
    }

    /**
     * Retrieve the implementation of the world
     *
     * \returns the distributed system implementation
     *
     */
    // FIXME: make a choice for the name
    Implementation& implementation() { return implementation_; }
    Implementation& provider() { return implementation_; }

  private:
    Implementation implementation_;
};

/**
 * Constructs a variable that is registered with `world`.
 *
 * \tparam T the type of the variable
 * \param world the distributed layer in which the variable is defined.
 *
 * \returns a newly constructed and registered variable
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

/**
 * Constructs a future, and registers it with `world`.
 *
 * \tparam T the type of the variable
 *
 * \param world the distributed layer in which the future is defined.
 *
 * \returns a newly constructed and registered future
 */
template <typename T, typename World>
typename World::template future_type<T> create_future(World& world) {
    return typename World::template future_type<T>(world);
}

/**
 * Constructs a message queue, and registers it with `world`.
 *
 * \tparam Tag the tag type of the queue
 * \tparam Conent the content type of the queue
 *
 * \param world the distributed layer in which the queue is defined.
 *
 * \returns a newly constructed and registered queue
 */
template <typename Tag, typename Content, typename World>
typename World::template queue_type<Tag, Content> create_queue(World& world) {
    return typename World::template queue_type<Tag, Content>(world);
}

} // namespace bulk
