#include <iostream>

#include <bulk/bulk.hpp>
#include <bulk/bsp/provider.hpp>

int main() {
    auto env = bulk::environment<bulk::bsp::provider>();

    env.spawn(env.available_processors(), [](auto world, int s, int p) {
        for (int t = 0; t < p; ++t) {
            bulk::send<int, int>(world, t, s, s);
        }

        world.sync();

        if (s == 0) {
            for (auto message : bulk::messages<int, int>(world)) {
                std::cout << message.tag << ", " << message.content
                          << std::endl;
            }
        }
    });

    return 0;
}
