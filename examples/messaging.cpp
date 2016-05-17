#include <iostream>

#include <bulk/bulk.hpp>
#include <bulk/bsp/provider.hpp>

int main() {
    auto env = bulk::environment<bulk::bsp::provider>();

    env.spawn(env.available_processors(),
              [](bulk::environment<bulk::bsp::provider>::world_type world,
                 int s, int p) {
                  for (int t = 0; t < p; ++t) {
                      world.send<int, int>(t, s, s);
                  }

                  world.sync();

                  if (s == 0) {
                      for (auto message : world.messages<int, int>()) {
                          std::cout << message.tag << ", " << message.content
                                    << std::endl;
                      }
                  }
              });

    return 0;
}
