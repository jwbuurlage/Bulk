#include <bulk/backends/epiphany/epiphany.hpp>
#include <bulk/world.hpp>

using bulk::epiphany::world;

int main() {
    int s = world.processor_id();
    int p = world.active_processors();
    world.sync();

    get_raw_time();
    get_raw_time();
    unsigned int t1 = get_raw_time();

    bulk::epiphany::print("Nothing time: %d", t1);

    world.barrier();
    get_raw_time();
    get_raw_time();
    world.barrier();
    unsigned int t2 = get_raw_time();

    bulk::epiphany::print("Barrier time: %d", t2);

    constexpr int nmax = 11;
    unsigned int* times = new unsigned int[nmax];
    for (int i = 0; i < nmax; ++i) {
        get_raw_time();
        for (int j = 0; j < i; ++j)
            world.barrier();
        unsigned int t = get_raw_time();
        times[i] = t;
    }

    for (int k = 0; k < p; k++) {
        world.barrier();
        if (k != s)
            continue;
        for (int i = 0; i < nmax; ++i) {
            bulk::epiphany::print("(%d,%d)", i, times[i]);
        }
    }

    delete[] times;

    world.sync();

    return 0;
}
