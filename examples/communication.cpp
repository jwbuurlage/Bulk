#include <iostream>

#include <bulk/bulk.hpp>
#include <bulk/bsp/provider.hpp>
#include <bulk/util/log.hpp>

int main() {
    auto env = bulk::environment<bulk::bsp::provider>();

    env.spawn(env.available_processors(), [](auto world, int s, int p) {
        auto a = bulk::create_var<int>(world);

        bulk::put(world.next_processor(), s, a);
        world.sync();

        BULK_IN_ORDER(std::cout << s << " <- " << a.value() << std::endl;)

        auto b = bulk::get(world.next_processor(), a);

        world.sync();

        BULK_IN_ORDER(std::cout << s << " -> " << b.value() << std::endl;)
    });

    return 0;
}
