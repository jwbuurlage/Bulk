#include <bulk/backends/mpi/mpi.hpp>
#include <bulk/bulk.hpp>
//#include <bulk/util/log.hpp>

#include "bulk_mpi_test_common.hpp"

extern bulk::environment<provider> env;

void test_communication() {
    env.spawn(env.available_processors(), [](auto world, int s, int p) {
        BULK_SECTION("Put") {
            // test `put` to single variable
            auto a = bulk::create_var<int>(world);

            bulk::put(world.next_processor(), s, a);
            world.sync();

            BULK_CHECK_ONCE(a.value() == ((s + p - 1) % p),
                            "wrong value after putting");
        }

        BULK_SECTION("Sugarized put") {
            // test `put` to single variable
            auto a = bulk::create_var<int>(world);

            a(world.next_processor()) = s;
            world.sync();

            BULK_CHECK_ONCE(a.value() == ((s + p - 1) % p),
                            "wrong value after sugarized putting");
        }

        BULK_SECTION("Put to self") {
            auto a = bulk::create_var<int>(world);

            bulk::put(s, s, a);
            world.sync();

            BULK_CHECK_ONCE(a.value() == s,
                            "wrong value after putting to self");
        }

        BULK_SECTION("Get from self") {
            auto a = bulk::create_var<int>(world);
            a = s;
            auto b = bulk::get(s, a);
            world.sync();

            BULK_CHECK_ONCE(b.value() == s,
                            "wrong value after putting to self");
        }

        BULK_SECTION("Put non-int") {
            // test `put` float to single variable
            auto a = bulk::create_var<float>(world);

            bulk::put(world.next_processor(), 1.0f, a);
            world.sync();

            BULK_CHECK_ONCE(a.value() == 1.0f,
                            "wrong value after putting float");
        }

        BULK_SECTION("Put multiple") {
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
        }

        BULK_SECTION("Put unequal") {
            int size = 5;

            // test `put` to multiple variables
            std::vector<decltype(bulk::create_var<int>(world))> xs;
            for (int i = 0; i < size; ++i)
                xs.push_back(bulk::create_var<int>(world));

            if (s == 0)
                for (int i = 1; i < p; ++i) {
                    for (int j = 0; j < size; ++j) {
                        bulk::put(i, i, xs[j]);
                    }
                }

            world.sync();

            auto a = bulk::create_future<int>(world);
            if (s == 0) a = bulk::get(p - 1, xs[size - 1]);

            world.sync();

            BULK_CHECK_ONCE(a.value() == p - 1,
                            "wrong value after heterogeneous puts and getting");
        }

        BULK_SECTION("Get") {
            auto b = bulk::create_var<int>(world);
            b.value() = s;
            world.sync();

            auto c = bulk::get(world.next_processor(), b);
            world.sync();

            BULK_CHECK_ONCE(c.value() == world.next_processor(),
                            "wrong value after getting");
        }

        BULK_SECTION("Sugarized get") {
            auto b = bulk::create_var<int>(world);
            b.value() = s;
            world.sync();

            auto c = b(world.next_processor()).get();
            world.sync();

            BULK_CHECK_ONCE(c.value() == world.next_processor(),
                            "wrong value after sugarized getting");
        }

        BULK_SECTION("Get multiple") {
            int size = 5;
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
        }

        BULK_SECTION("Coarray") {
            auto zs = bulk::create_coarray<int>(world, 10);
            zs(world.next_processor())[1] = s;

            world.sync();

            BULK_CHECK_ONCE(
                zs[1] == world.prev_processor(),
                "putting to remote coarray image gives wrong result");

            zs[3] = 2;

            BULK_CHECK_ONCE(zs[3] == 2,
                            "writing to local coarray gives wrong result");
        }

        BULK_SECTION("Coarray iteration") {
            auto xs = bulk::gather_all(world, s);
            int t = 0;
            for (auto x : xs) {
                BULK_CHECK_ONCE(x == t++, "gather operation failed");
            }
        }
    });
}
