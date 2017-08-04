#include "bulk/bulk.hpp"

#include "set_backend.hpp"

int main() {
    environment env;

    env.spawn(env.available_processors(), [](bulk::world& world) {
        int s = world.rank();
        int p = world.active_processors();

        world.log("Hello, world %d/%d", s, p);

        auto a = bulk::var<int>(world);
        a(world.next_rank()) = s;
        world.sync();
        // ... the local a is now updated
        world.log("%d got put %d", s, a.value());

        auto b = a(world.next_rank()).get();
        world.sync();
        // ... b.value() is now available

        // coarrays are distributed arrays, each processor has an array to which
        // other processors can write
        auto xs = bulk::coarray<int>(world, 10);
        xs(world.next_rank())[3] = s;

        // messages can be passed to queues that specify a tag type, and a
        // content type
        auto q = bulk::queue<int, float>(world);
        for (int t = 0; t < p; ++t) {
            q(t).send(s, 3.1415f); // send (s, pi) to processor t
        }
        world.sync();

        // Messages are now available in q
        for (auto [tag, content] : q) {
            world.log("%d got sent %d, %f", s, tag, content);
        }
    });

    return 0;
}
