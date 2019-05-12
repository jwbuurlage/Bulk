#pragma once

/**
 * \file environment.hpp
 *
 * This header provides an object that can be used to retrieve the necessary
 * information about the parallel environment of a layer.
 */

#include <functional>

namespace bulk {

class world;

/**
 * This object represents a parallel environment, and can be used to obtain
 * information of the parallel system.
 */
class environment {
  public:
    virtual ~environment() = default;

    /**
     * Start an SPMD section on a given number of processors.
     *
     * \param processors the number of processors to run on
     * \param spmd the spmd function that gets run on each (virtual) processor
     */
    virtual void spawn(int processors, std::function<void(bulk::world&)> spmd) = 0;

    /**
     * Get the total number of processors available on the system
     *
     * \returns the number of available processors
     */
    virtual int available_processors() const = 0;

    /**
     * Set a callback that receives output from `world::log` and
     * `world::log_direct`. If no callback is set, the results will
     * be printed to standard output.
     *
     * The first argument to the callback is the processor id
     * and the second argument is the logged message.
     *
     * The callback must be set before calling `spawn`.
     *
     * On multicore systems the callback could be called from a different
     * thread. It is guaranteed that there will not be two calls to the
     * callback at the same time, so there is no need for further
     * synchronization or locking mechanisms.
     */
    virtual void set_log_callback(std::function<void(int, const std::string&)> f) = 0;
};

} // namespace bulk
