#include "bulk/bulk.hpp"

#include "set_backend.hpp"

int main() {
    environment env;

    env.spawn(env.available_processors(), [](bulk::world& world) {
        int s = world.processor_id();

        bulk::queue<int, int> q(world);
        q(world.next_processor()).send(s, s + 1);

        world.sync();

        for (auto [tag, content] : q)
            world.log("%d got sent %d, %d", s, tag, content);
    });

    return 0;
}
