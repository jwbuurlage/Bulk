#include <vector>
#include <numeric>

#include <catch.hpp>

#include <bulk/bulk.hpp>
#include <bulk/bsp/provider.hpp>
#include <bulk/util/log.hpp>

#include "bulk_test_common.hpp"

using provider = bulk::bsp::provider;


TEST_CASE("basic communication", "[communication]") {
    SECTION("put") {
        auto env = bulk::environment<provider>();

        env.spawn(env.available_processors(), [](auto world, int s, int p) {
            auto a = bulk::create_var<int>(world);

            bulk::put(world.next_processor(), s, a);
            world.sync();

            BULK_CHECK_ONCE(a.value() == ((s + p - 1) % p));
        });
    }

    SECTION("multiple put") {
        auto env = bulk::environment<provider>();

        env.spawn(env.available_processors(), [](auto world, int s, int p) {
            int size = 5;
            std::vector<bulk::var<int, decltype(world)>> xs;
            for (int i = 0; i < size; ++i)
                xs.push_back(bulk::create_var<int>(world));

            for (int i = 0; i < size; ++i) {
                bulk::put(world.next_processor(), s + i, xs[i]);
            }

            world.sync();

            for (int i = 0; i < size; ++i) {
                BULK_CHECK_ONCE(xs[i].value() == ((s + p - 1) % p) + i);
            }
        });
    }

    SECTION("get") {
        auto env = bulk::environment<provider>();

        env.spawn(env.available_processors(), [](auto world, int s, int) {
            auto a = bulk::create_var<int>(world);
            a.value() = s;

            auto b = bulk::get(world.next_processor(), a);
            world.sync();

            BULK_CHECK_ONCE(b.value() == world.next_processor());
        });
    }

    SECTION("multiple get") {
        auto env = bulk::environment<provider>();

        env.spawn(env.available_processors(), [](auto world, int s, int) {
            auto x = bulk::create_var<int>(world);
            x.value() = s;

            int size = 5;
            std::vector<bulk::future<int, decltype(world)>> ys;
            for (int i = 0; i < size; ++i) {
                ys.push_back(bulk::get(world.next_processor(), x));
            }

            world.sync();

            for (auto& y : ys) {
                BULK_CHECK_ONCE(y.value() == world.next_processor());
            }
        });
    }

    SECTION("message passing") {
        auto env = bulk::environment<provider>();

        env.spawn(
            env.available_processors(),
            [](bulk::environment<provider>::world_type world, int s, int p) {
                for (int t = 0; t < p; ++t) {
                bulk::send<int, int>(world,t, s, s);
                }

                world.sync();

                std::vector<int> contents;
                for (auto message : bulk::messages<int, int>(world)) {
                    contents.push_back(message.content);
                }
                std::sort(contents.begin(), contents.end());

                std::vector<int> compare_result(p);
                std::iota(compare_result.begin(), compare_result.end(), 0);

                BULK_CHECK_ONCE(compare_result == contents);
            });
    }

    SECTION("coarrays") {
        auto env = bulk::environment<provider>();

        env.spawn(
            env.available_processors(),
            [](bulk::environment<provider>::world_type world, int s, int) {
                auto xs = bulk::create_coarray<int>(world, 10);
                xs(world.next_processor())[1] = s;

                world.sync();

                BULK_CHECK_ONCE(xs[1] == world.prev_processor());

                xs[3] = 2;

                BULK_CHECK_ONCE(xs[3] == 2);
            });
    }
}
