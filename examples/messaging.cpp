#include "set_backend.hpp"
#include <bulk/bulk.hpp>

int main() {
    bulk::environment<provider> env;

    env.spawn(env.available_processors(), [](auto world, int s, int p) {
        auto q = bulk::create_queue<int, int>(world);
        for (int t = 0; t < p; ++t)
            q(t).send(s, s);

        world.sync();

        for (auto& msg : q)
            world.log("%d got sent %d, %d\n", s, msg.tag, msg.content);
    });

    return 0;
}
