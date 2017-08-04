#include "bulk/bulk.hpp"

#include "set_backend.hpp"

int main() {
    environment env;

    env.spawn(env.available_processors(), [](bulk::world& world) {
        int s = world.rank();
        int p = world.active_processors();

        bulk::var<int> a(world);

        a(world.next_rank()) = s;
        world.sync();

        world.log("%d/%d <- %d", s, p, a.value());

        auto b = a(world.next_rank()).get();

        world.sync();

        world.log("%d/%d -> %d", s, p, b.value());
    });

    return 0;
}
