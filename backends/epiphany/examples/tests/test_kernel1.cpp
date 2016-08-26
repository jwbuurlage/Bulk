#include <bulk/world.hpp>
#include <bulk/coarray.hpp>
#include <bulk/backends/epiphany/epiphany.hpp>

using namespace bulk::epiphany;

int main() {
    auto a = bulk::create_var<int>(world);

    a(world.next_processor()) = world.processor_id();
    world.sync();

    if (a.value() == world.prev_processor())
        print("SUCCESS");
    else
        print("FAIL");

    world.sync();

    return 0;
}
