#include <bulk/world.hpp>
#include <bulk/variable.hpp>
#include <bulk/coarray.hpp>
#include <bulk/epiphany/epiphany.hpp>
#include <vector>

int s, p;

#if 0
void example_var() {
    bulk::var<int> a;
    
    // local explicit
    a.value() = 5;

    // local implicit
    a = 3;
}

void example_var_array() {
    constexpr int size = 16;
    bulk::var<float[size]> b;

    // Explicit local operators
    for (int i = 0; i < size; i++)
        b.value()[i] = 1.0f;

    // Implicit (local) cast operators
    b[0] = -18.0f;

    world.sync();

    // Remote access operator ()
    for (int k = 0; k < p; k++)
        b(k)[s] = 100.0f * ((float)s) + ((float)k);

    world.sync();

    bulk::epiphany::print("first three values of b: %f, %f, %f", b[0], b[1], b[2]);
}

void example_coarray() {
    int size = 2 * p;
    bulk::coarray<int> c(size);

    // Explicit local
    for (int i = 0; i < size; i++)
        c.value()[i] = i;

    // Implicit local
    c[3] = 3.2f;

}
#endif

int main() {
    s = world.processor_id();
    p = world.active_processors();

    bulk::epiphany::print("Welcome to Bulk!");

    world.sync();

    bulk::epiphany::print("Using variables for communication!");

#if 0
    example_var();
    example_var_array();
    example_coarray();
#endif

    world.sync();

    return 0;
}
