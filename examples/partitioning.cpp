#include <iostream>

#include <bulk/bulk.hpp>
#include "set_backend.hpp"

int main() {
    bulk::environment<provider> env;

    env.spawn(env.available_processors(), [](auto world, int s, int p) {
        auto part = bulk::cyclic_partitioning<2>({2, 2}, {200, 200});
        auto xs =
            bulk::partitioned_array<int, 2, decltype(world)>(
                world, part);

        xs.local(0, 0) = s;
        xs.local(1, 1) = s + 1;

        auto glob = xs.global(1, 1).get();
        world.sync();

        if (s == 0) {
            std::cout << glob.value() << "\n";
            std::cout << xs.local(1, 1) << "\n";
        }
    });

    return 0;
}
