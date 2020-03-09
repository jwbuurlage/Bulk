#include <chrono>
#include <thread>

#include "bulk_test_common.hpp"
#include "set_backend.hpp"
#include <bulk/bulk.hpp>

extern environment env;

void test_algorithm() {
    env.spawn(env.available_processors(), [](auto& world) {
        int s = world.rank();
        int p = world.active_processors();

        BULK_SKIP_SECTION_IF("Algorithm", p <= 1);

        BULK_SECTION("foldl") {
            bulk::var<int> a(world, s + 1);

            auto fold_result =
            bulk::foldl(a, [](auto& lhs, auto& rhs) { lhs += rhs; });

            BULK_CHECK(fold_result == p * (p + 1) / 2,
                       "accumulating  1 + 2 + ... + p");

            bulk::coarray<int> xs(world, 2);
            xs[0] = 2 * s + 1;
            xs[1] = 2 * s + 2;

            auto co_fold_result =
            bulk::foldl(xs, [](auto& lhs, auto& rhs) { lhs += rhs; });

            BULK_CHECK(co_fold_result == 2 * p * (2 * p + 1) / 2,
                       "accumulating  1 + 2 + ... + 2 p");
        }

        BULK_SECTION("foldl_each") {
            bulk::coarray<int> xs(world, 2);
            xs[0] = s + 1;
            xs[1] = s;

            auto f1 =
            bulk::foldl_each(xs, [](auto& lhs, auto& rhs) { lhs += rhs; });

            BULK_CHECK(f1[0] == p * (p + 1) / 2 && f1[1] == (p - 1) * p / 2,
                       "foldl_each");

            bulk::coarray<int> zs(world, s, s + 1);

            auto f2 =
            bulk::foldl_each(zs, [](auto& lhs, auto& rhs) { lhs += rhs; });

            BULK_CHECK(f2.empty(), "calling foldl_each with non-constant size");
        }
    });
}
