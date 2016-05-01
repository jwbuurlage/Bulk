#include <bulk/bsp/bulk.hpp>
#include <bulk/util/log.hpp>

#include <iostream>

int main() {
    auto bulk = bulk::bulk_bsp();

    bulk.spawn(bulk.available_processors(), [bulk](int s, int p) {
        BULK_IN_ORDER(
            std::cout << "Hello, world " << s << "/" << p << std::endl;
        )
    });

    return 0;
}
