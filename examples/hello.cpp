#include <iostream>

#include <bulk/bsp/bulk.hpp>
#include <bulk/util/log.hpp>

int main() {
    auto hub = bulk::bsp_hub();

    hub.spawn(hub.available_processors(), [&hub](int s, int p) {
        BULK_IN_ORDER(
            std::cout << "Hello, world " << s << "/" << p << std::endl;
        )
    });

    return 0;
}
