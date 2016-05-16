#pragma once

#include <vector>

#include <bulk/communication.hpp>


namespace bulk {

// convenience functions
template <typename T, typename World, typename TFunc>
T reduce(var<T, World>& variable, TFunc f, T start_value = 0) {
    auto& world = variable.world();

    T result = start_value;
    std::vector<bulk::future<T, World>> images;
    for (int t = 0; t < world.active_processors(); ++t) {
        images.push_back(bulk::get<T>(t, variable));
    }
    world.sync();
    for (int t = 0; t < world.active_processors(); ++t) {
        f(result, images[t].value());
    }
    return result;
}

} // namespace bulk
