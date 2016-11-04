#include <bulk/bulk.hpp>
#include "set_backend.hpp"

int main() {
    bulk::environment<provider> env;

    env.spawn(env.available_processors(), [](auto world, int s, int p) {
        auto a = bulk::create_var<int>(world);

        a(world.next_processor()) = s;
        world.sync();

        world.log("%d/%d <- %d\n", s, p, a.value());

        auto b = a(world.next_processor()).get();

        world.sync();

        world.log("%d/%d -> %d\n", s, p, b.value());
    });

    return 0;
}
