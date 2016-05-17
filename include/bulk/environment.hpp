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
 * \brief This objects encodes the parallel environment of this layer, and
 * provides information on the system.
 */
template <class Provider>
class environment {
  public:
    using world_provider = typename Provider::world_provider_type;
    using world_type = world<world_provider>;

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
     * Retrieve the provider of the distributed system
     * 
     * \returns the distributed system provider
    */
    Provider& provider() { return provider_; }

  private:
    Provider provider_;
};

} // namespace bulk
