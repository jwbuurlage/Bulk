#include <bulk/world.hpp>
#include <bulk/coarray.hpp>
#include <bulk/backends/epiphany/epiphany.hpp>
#include <vector>

using namespace bulk::epiphany;

int main() {
    bool fail = false;
    int s = world.processor_id();

    auto a = bulk::create_var<int>(world);
    auto b = bulk::create_var<int>(world);

    a(world.next_processor()) = s + 3;
    b(world.prev_processor()) = s + 17;

    world.sync();

    if (a != world.prev_processor() + 3) {
        print("FAIL: a = %d", (int)a);
        fail = true;
    }
    if (b != world.next_processor() + 17) {
        print("FAIL: b = %d", (int)b);
        fail = true;
    }

    // Try variables in a vector
    //
    // This also tests the move constructor

    int size = 5;

    std::vector<decltype(bulk::create_var<int>(world))> xs;

    for (int i = 0; i < size; ++i)
        xs.push_back(bulk::create_var<int>(world));

    for (int i = 0; i < size; ++i)
        xs[i](world.next_processor()) = s + i;

    world.sync();

    for (int i = 0; i < size; ++i) {
        if (xs[i].value() != world.prev_processor() + i) {
            fail = true;
            print("FAIL: xs[%d] == %d != %d", i, xs[i].value(),
                  world.prev_processor() + i);
        }
    }

    if (!fail)
        print("SUCCESS");

    world.sync();

    return 0;
}
