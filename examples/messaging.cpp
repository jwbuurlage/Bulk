#include "bulk/bulk.hpp"

#include "set_backend.hpp"

int main() {
    environment env;

    env.spawn(env.available_processors(), [](bulk::world& world, int s, int p) {
        bulk::queue<int, int> q(world);
        for (int t = 0; t < p; ++t)
            q(t).send(s, s);

        world.sync();

        for (auto& msg : q)
            world.log("%d got sent %d, %d", s, msg.tag, msg.content);
    });

    return 0;
}
