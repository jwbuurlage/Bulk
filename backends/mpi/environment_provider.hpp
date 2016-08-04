#pragma once

#include <mpi.h>

#include "backend.hpp"

namespace bulk {
namespace mpi {

/**
 * This class defines the outer layer of the MPI backend. Within a single run,
 * only a single provider can be created or destroyed (this is a limitation of
 * MPI).
 */
class provider {
   public:
    using world_provider_type = world_provider;

    provider() { MPI_Init(nullptr, nullptr); }

    ~provider() { MPI_Finalize(); }

    void spawn(int processors,
               std::function<void(bulk::world<backend>, int, int)> spmd) {
        bulk::world<backend> world;
        spmd(world, world.processor_id(), world.active_processors());
    }

    int available_processors() const {
        int world_size;
        MPI_Comm_size(MPI_COMM_WORLD, &world_size);
        return world_size;
    }
};

}  // namespace mpi
}  // namespace bulk
