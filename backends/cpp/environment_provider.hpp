#pragma once
#include "backend.hpp"
#include <thread>
#include <vector>
#include <iostream>

namespace bulk {
namespace cpp {

class provider {
  public:
    provider() {}

    ~provider() {}

    void spawn(int processors,
               std::function<void(bulk::world<backend>, int, int)> spmd) {
        // Thread objects
        std::vector<std::thread> threads;

        // Shared world_state object
        world_state state(processors);

        // Create the threads and thereby start them
        for (int i = 0; i < processors; i++) {
            bulk::world<backend> world;
            world.implementation().init_(&state, i, processors);
            // copy or move world object into thread
            threads.push_back(std::thread(spmd, world, i, processors));
        }

        // Wait for the threads to finish
        for (auto& t : threads)
            t.join();
    }

    int available_processors() const {
        return std::thread::hardware_concurrency();
    }
};

} // namespace mpi
} // namespace bulk
