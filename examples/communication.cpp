#include <iostream>

#include <bulk/bsp/bulk.hpp>
#include <bulk/util/log.hpp>


int main() {
    auto hub = bulk::bsp_hub();

    hub.spawn(hub.available_processors(), [&hub](int s, int) {
        auto a = hub.create_var<int>();

        hub.put(hub.next_processor(), s, a);
        hub.sync();

        BULK_IN_ORDER(std::cout << s << " <- " << a.value() << std::endl;)

        auto b = hub.get(hub.next_processor(), a);

        hub.sync();

        BULK_IN_ORDER(std::cout << s << " -> " << b.value() << std::endl;)
    });

    return 0;
}
