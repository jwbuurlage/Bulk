#include "set_backend.hpp"
#include <bulk/bulk.hpp>

int main() {
    bulk::environment<provider> env;

    env.spawn(env.available_processors(), [](auto world, int s, int p) {
        world.log("Hello, world %d/%d\n", s, p);
    });

    return 0;
}
