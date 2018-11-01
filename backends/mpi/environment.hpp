#pragma once

#include <iostream>

#include <bulk/environment.hpp>
#include <mpi.h>

#include "world.hpp"

namespace bulk::mpi {

/**
 * This class defines the outer layer of the MPI backend. Within a single run,
 * only a single provider can be created or destroyed (this is a limitation of
 * MPI).
 */
class environment : public bulk::environment {
  public:
    environment() { MPI_Init(nullptr, nullptr); }
    ~environment() { MPI_Finalize(); }

    void spawn(int processors,
               std::function<void(bulk::world&)> spmd) override final {
        if (processors < available_processors()) {
            std::cout << "Running with fewer processors than available is not "
                         "yet implemented in MPI.\n";
            return;
        }
        bulk::mpi::world world;
        spmd(world);
    }

    int available_processors() const override final {
        int world_size;
        MPI_Comm_size(MPI_COMM_WORLD, &world_size);
        return world_size;
    }

    void set_log_callback(
        std::function<void(int, const std::string&)> f) override final {
        // do nothing
        (void)f;
    }
};

} // namespace bulk::mpi
