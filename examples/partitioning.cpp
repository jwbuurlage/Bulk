#include <iostream>

#include <bulk/bulk.hpp>
#include "set_backend.hpp"

int main() {
    environment env;

    env.spawn(env.available_processors(), [](bulk::world& world, int s, int) {
        auto part = bulk::cyclic_partitioning<2>(world, {200, 200}, {2, 2});
        auto xs = bulk::partitioned_array<int, 2, 2>(world, part);

        xs.local({0, 0}) = s;
        xs.local({1, 1}) = s + 1;

        auto glob = xs.global({1, 1}).get();
        world.sync();

        if (s == 0) {
            std::cout << glob.value() << "\n";
            std::cout << xs.local({1, 1}) << "\n";
        }
    });

    return 0;
}
