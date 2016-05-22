#include <bulk/world.hpp>
#include <bulk/coarray.hpp>
#include <bulk/epiphany/epiphany.hpp>
#include <vector>

int s, p;

void example_var() {
    auto a = bulk::create_var<int>(world);
    
    // local explicit
    a.value() = 5;

    // local implicit
    a = 3;

    world.sync();

    // remote
    a(world.next_processor()) = s;

    world.sync();

    bulk::epiphany::print("value of a is %d", a.value());
}

void example_coarray() {
    int size = 2 * p;
    auto c = bulk::create_coarray<int>(world, size);

    // Local
    c[3] = 3.2f;
}

int main() {
    s = world.processor_id();
    p = world.active_processors();

    bulk::epiphany::print("Welcome to Bulk!");

    world.sync();

    bulk::epiphany::print("Using variables for communication!");

    example_var();
    example_coarray();

    world.sync();

    return 0;
}
