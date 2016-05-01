#include <iostream>

#include <bulk/bsp/bulk.hpp>
#include <bulk/util/log.hpp>

namespace bulk = bulk_bsp;


int main() {
    auto center = bulk::center();

    center.spawn(center.available_processors(), [&center](int s, int p) {
        BULK_IN_ORDER(
            std::cout << "Hello, world " << s << "/" << p << std::endl;
        )
    });

    return 0;
}
