#include <iostream>

#include <bulk/hub.hpp>
#include <bulk/variable.hpp>
#include <bulk/communication.hpp>
#include <bulk/bsp/bulk.hpp>
#include <bulk/util/log.hpp>


int main() {
    auto hub = bulk::hub<bulk::bsp::provider>();

    hub.spawn(hub.available_processors(), [&hub](int s, int) {
        auto a = bulk::create_var<int>(hub);

        bulk::put(hub.next_processor(), s, a);
        hub.sync();

        BULK_IN_ORDER(std::cout << s << " <- " << a.value() << std::endl;)

        auto b = bulk::get(hub.next_processor(), a);

        hub.sync();

        BULK_IN_ORDER(std::cout << s << " -> " << b.value() << std::endl;)
    });

    return 0;
}
