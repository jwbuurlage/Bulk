#include "bulk/bulk.hpp"

#include "set_backend.hpp"

int main() {
    environment env;

    env.spawn(env.available_processors(), [](bulk::world& world) {
        int s = world.processor_id();
        int p = world.active_processors();

        world.log("Hello, world %d/%d", s, p);

        auto a = bulk::var<int>(world);
        a(world.next_processor()) = s;
        world.sync();
        // ... the local a is now updated

        auto b = a(world.next_processor()).get();
        world.sync();
        // ... b.value() is now available

        // coarrays are distributed arrays, each processor has an array to which
        // other processors can write
        auto xs = bulk::coarray<int>(world, 10);
        xs(world.next_processor())[3] = s;

        // messages can be passed to queues that specify a tag type, and a
        // content type
        auto q = bulk::queue<int, float>(world);
        for (int t = 0; t < p; ++t) {
            q(t).send(s, 3.1415f); // send (s, pi) to processor t
        }
        world.sync();

        // Messages are now available in q
        for (auto& msg : q) {
            world.log("%d got sent %d, %f", s, msg.tag, msg.content);
        }
    });

    return 0;
}
