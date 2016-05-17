#include <iostream>

#include <bulk/bulk.hpp>
#include <bulk/bsp/provider.hpp>


int main() {
    auto env = bulk::environment<bulk::bsp::provider>();

    env.spawn(env.available_processors(), [](auto world, int s, int p) {
        BULK_IN_ORDER(
            std::cout << "Hello, world " << s << "/" << p << std::endl;
        )
    });

    return 0;
}
