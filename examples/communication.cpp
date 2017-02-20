#include <bulk/bulk.hpp>
#include "set_backend.hpp"

int main() {
    environment env;

    env.spawn(env.available_processors(), [](bulk::world& world, int s, int p) {
        bulk::var<int> a(world);

        a(world.next_processor()) = s;
        world.sync();

        world.log("%d/%d <- %d\n", s, p, a.value());

        auto b = a(world.next_processor()).get();

        world.sync();

        world.log("%d/%d -> %d\n", s, p, b.value());
    });

    return 0;
}
