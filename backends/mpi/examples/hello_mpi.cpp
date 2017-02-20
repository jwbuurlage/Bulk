#include <iostream>

#include "../mpi.hpp"
#include <bulk/bulk.hpp>

int main() {
    bulk::mpi::environment env;

    env.spawn(env.available_processors(), [](auto& world, int, int) {
        world.log("Hi!");
    });

    return 0;
}
