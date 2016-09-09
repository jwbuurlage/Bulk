#pragma once
#include "backend.hpp"
#include <algorithm>
#include <iostream>
#include <thread>
#include <vector>

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
        state.log_callback = log_callback;

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

        // Print leftover log messages
        auto& logs = state.logs;
        std::sort(logs.begin(), logs.end());
        if (state.log_callback == nullptr) {
            for (auto& log : logs)
                std::cout << log.second;
            std::cout << std::flush;
        } else {
            for (auto& log : logs)
                state.log_callback(log.first, log.second);
        }
        logs.clear();
    }

    int available_processors() const {
        return std::thread::hardware_concurrency();
    }

    void set_log_callback(std::function<void(int, const std::string&)> f) {
        log_callback = f;
    }

  private:
    std::function<void(int, const std::string&)> log_callback;
};

} // namespace mpi
} // namespace bulk
