#include <bulk/world.hpp>
#include <bulk/coarray.hpp>
#include <bulk/backends/epiphany/epiphany.hpp>
#include <vector>

using bulk::epiphany::world;

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

void example_extmem() {
    int count = 2 * p;
    int* mem = (int*)bulk::epiphany::ext_malloc(count * sizeof(int));

    for (int i = 0; i < count; i++)
        mem[i] = s;

    bulk::epiphany::free(mem);
}

void example_stream() {
    bulk::epiphany::stream stream(s);

    if (stream) {
        int count = 100;
        int* buf = new int[count];

        for (int i = 0; i < count; ++i)
            buf[i] = -1;

        int read = stream.read(buf, count * sizeof(int), true);
        count = read / sizeof(int);

        int sum = 0;
        for (int i = 0; i < count; i++)
            sum += buf[i];

        delete[] buf;

        bulk::epiphany::print("Stream sum of %d elements: %d", count, sum);

        // Go to start
        stream.seek_abs(0);
        // Write the sum to the start of the stream
        stream.write(&sum, sizeof(int), false);

        stream.close();
    } else {
        bulk::epiphany::print("Unable to open stream.");
    }
}

int main() {
    s = world.processor_id();
    p = world.active_processors();

    bulk::epiphany::print("Welcome to Bulk!");

    world.sync();

    bulk::epiphany::print("Using variables for communication!");

    example_var();
    example_coarray();

    bulk::epiphany::print("Using external memory!");
    example_extmem();

    world.sync();
    example_stream();

    world.sync();

    return 0;
}
