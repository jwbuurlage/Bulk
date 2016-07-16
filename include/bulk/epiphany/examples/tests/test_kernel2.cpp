#include <bulk/world.hpp>
#include <bulk/coarray.hpp>
#include <bulk/epiphany/epiphany.hpp>
#include <vector>

using namespace bulk::epiphany;

int main() {
    int s = world.processor_id();

    int size = 5;

    std::vector<decltype(bulk::create_var<int>(world))> xs;

    for (int i = 0; i < size; ++i)
        xs.push_back(bulk::create_var<int>(world));

    world.sync();

    for (int i = 0; i < size; ++i)
        xs[i](world.next_processor()) = s + i;

    world.sync();

    for (int i = 0; i < size; ++i) {
        if (xs[i].value() == world.prev_processor() + i)
            print("SUCCESS %d", i);
        else
            print("FAIL %d", i);
    }

    world.sync();

    return 0;
}
