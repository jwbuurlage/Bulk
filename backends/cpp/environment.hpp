#pragma once
#include "world.hpp"
#include <bulk/environment.hpp>
#include <algorithm>
#include <iostream>
#include <thread>
#include <vector>

namespace bulk {
namespace cpp {

class environment : public bulk::environment {
  public:
    environment() {}

    ~environment() {}

    void spawn(int processors,
                        std::function<void(bulk::world&, int, int)> spmd) override {
        // Thread objects
        std::vector<std::thread> threads;

        // Shared world_state object
        world_state state(processors);
        state.log_callback = log_callback;

        // Create the threads and thereby start them
        std::vector<bulk::cpp::world> ws;
        ws.reserve(processors);
        for (int i = 0; i < processors; i++) {
            ws.emplace_back(&state, i, processors);
            threads.push_back(std::thread(spmd, std::ref(ws.back()), i, processors));
        }

        // Wait for the threads to finish
        for (auto& t : threads)
            t.join();

        // Print leftover log messages
        auto& logs = state.logs;
        std::stable_sort(logs.begin(), logs.end());
        if (state.log_callback == nullptr) {
            for (auto& log : logs)
                std::cout << log.second << '\n';
            std::cout << std::flush;
        } else {
            for (auto& log : logs)
                state.log_callback(log.first, log.second);
        }
        logs.clear();
    }

    int available_processors() const override {
        return std::thread::hardware_concurrency();
    }

    void set_log_callback(std::function<void(int, const std::string&)> f) override {
        log_callback = f;
    }

  protected:
    std::function<void(int, const std::string&)> log_callback;
};

} // namespace cpp
} // namespace bulk
