#include <bulk/world.hpp>
#include <bulk/coarray.hpp>
#include <bulk/backends/epiphany/epiphany.hpp>
#include <vector>

using namespace bulk::epiphany;

int main() {
    int s = world.processor_id();

    auto xs = bulk::create_coarray<int>(world, 10);
    xs(world.next_processor())[1] = s;

    world.sync();

    if (xs[1] == world.prev_processor())
        print("SUCCESS");
    else
        print("FAIL");

    xs[3] = 2;

    if (xs[3] == 2)
        print("SUCCESS");
    else
        print("FAIL");

    world.sync();

    return 0;
}
