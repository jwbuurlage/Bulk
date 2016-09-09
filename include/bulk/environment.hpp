#pragma once

/**
 * \file environment.hpp
 *
 * This header provides an object that can be used to retrieve the necessary
 * information about the parallel environment of a layer.
 */

#include <functional>
#include <memory>
#include <vector>


namespace bulk {

template <class WorldProvider>
class world;

/**
 * This object encodes the parallel environment of this layer, and
 * provides information on the system.
 */
template <class Provider>
class environment {
  public:
    /**
     * Start an spmd section on a given number of processors.
     *
     * \param processors the number of processors to run on
     * \param spmd the spmd function that gets run on each (virtual) processor
     */
    template <typename Func>
    void spawn(int processors, Func spmd) {
        provider_.spawn(processors, spmd);
    }

    /**
     * Retrieve the total number of processors available on the system
     *
     * \returns the number of available processors
    */
    int available_processors() const {
        return provider_.available_processors();
    }

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
    void set_log_callback(std::function<void(int, const std::string&)> f) {
        provider_.set_log_callback(f);
    }

    /**
     * Retrieve the provider of the distributed system
     *
     * \returns the distributed system provider
    */
    Provider& provider() { return provider_; }

  private:
    Provider provider_;
};

} // namespace bulk
