#include <bulk/backends/mpi/mpi.hpp>
#include <bulk/bulk.hpp>
//#include <bulk/util/log.hpp>

#include "bulk_mpi_test_common.hpp"

extern bulk::environment<provider> env;

void test_communication() {
    env.spawn(env.available_processors(), [](auto world, int s, int p) {
        // test `put` to single variable
        auto a = bulk::create_var<int>(world);

        bulk::put(world.next_processor(), s, a);
        world.sync();

        BULK_CHECK_ONCE(a.value() == ((s + p - 1) % p),
                        "wrong value after putting");

        int size = 5;

        // test `put` to multiple variables
        std::vector<decltype(bulk::create_var<int>(world))> xs;
        for (int i = 0; i < size; ++i)
            xs.push_back(bulk::create_var<int>(world));

        for (int i = 0; i < size; ++i) {
            bulk::put(world.next_processor(), s + i, xs[i]);
        }

        world.sync();

        for (int i = 0; i < size; ++i) {
            BULK_CHECK_ONCE(
                xs[i].value() == ((s + p - 1) % p) + i,
                "wrong value after multiple puts to array of variables");
        }

        world.sync();

        auto b = bulk::create_var<int>(world);
        b.value() = s;

        auto c = bulk::get(world.next_processor(), b);
        world.sync();

        BULK_CHECK_ONCE(c.value() == world.next_processor(),
                        "wrong value after getting");

        auto x = bulk::create_var<int>(world);
        x.value() = s;

        std::vector<bulk::future<int, decltype(world)>> ys;
        for (int i = 0; i < size; ++i) {
            ys.push_back(bulk::get(world.next_processor(), x));
        }

        world.sync();

        for (auto& y : ys) {
            BULK_CHECK_ONCE(y.value() == world.next_processor(),
                            "wrong value after getting multiple");
        }

        auto zs = bulk::create_coarray<int>(world, 10);
        zs(world.next_processor())[1] = s;

        world.sync();

        BULK_CHECK_ONCE(zs[1] == world.prev_processor(),
                        "putting to remote coarray image gives wrong result");

        zs[3] = 2;

        BULK_CHECK_ONCE(zs[3] == 2,
                        "writing to local coarray gives wrong result");
    });
}
