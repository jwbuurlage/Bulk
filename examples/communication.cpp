#include <iostream>

#include <bulk/bsp/bulk.hpp>
#include <bulk/util/log.hpp>

namespace bulk = bulk_bsp;

int main() {
    auto center = bulk::center();

    center.spawn(center.available_processors(), [&center](int s, int) {
        bulk::var<int> a;

        center.put(center.next_processor(), s, a);
        center.sync();

        BULK_IN_ORDER(std::cout << s << " <- " << a.value() << std::endl;)

        auto b = center.get<int>(center.next_processor(), a);

        center.sync();

        BULK_IN_ORDER(std::cout << s << " -> " << b.value() << std::endl;)
    });

    return 0;
}
