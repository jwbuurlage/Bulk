#include "set_backend.hpp"
#include <bulk/bulk.hpp>

int main() {
    environment env;

    env.spawn(env.available_processors(), [](bulk::world& world, int s, int p) {
        world.log("Hello, world %d/%d\n", s, p);
    });

    return 0;
}
