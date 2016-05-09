#pragma once

#include <vector>

#include <bulk/communication.hpp>


namespace bulk {

// convenience functions
template <typename T, typename Hub, typename TFunc>
T reduce(var<T, Hub>& variable, TFunc f, T start_value = 0) {
    auto& hub = variable.hub();

    T result = start_value;
    std::vector<bulk::future<T, Hub>> images;
    for (int t = 0; t < hub.active_processors(); ++t) {
        images.push_back(bulk::get<T>(t, variable));
    }
    hub.sync();
    for (int t = 0; t < hub.active_processors(); ++t) {
        f(result, images[t].value());
    }
    return result;
}

} // namespace bulk
