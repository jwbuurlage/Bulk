#include <iostream>

#include <bulk/bulk.hpp>
#include <bulk/bsp/bulk.hpp>


int main() {
    auto hub = bulk::hub<bulk::bsp::provider>();

    hub.spawn(hub.available_processors(), [&hub](int s, int p) {
        BULK_IN_ORDER(
            std::cout << "Hello, world " << s << "/" << p << std::endl;
        )
    });

    return 0;
}
