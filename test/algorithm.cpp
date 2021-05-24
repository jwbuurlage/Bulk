#include <bulk/bulk.hpp>
#include <chrono>
#include <limits>
#include <thread>

#include "bulk_test_common.hpp"
#include "set_backend.hpp"

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

      auto f1 = bulk::foldl_each(xs, [](auto& lhs, auto& rhs) { lhs += rhs; });

      BULK_CHECK(f1[0] == p * (p + 1) / 2 && f1[1] == (p - 1) * p / 2,
                 "foldl_each");

      bulk::coarray<int> zs(world, s, s + 1);

      auto f2 = bulk::foldl_each(zs, [](auto& lhs, auto& rhs) { lhs += rhs; });

      BULK_CHECK(f2.empty(), "calling foldl_each with non-constant size");
    }

    BULK_SECTION("fold aliases") {
      BULK_CHECK(bulk::max(world, s + 1) == p, "max of local values");
      BULK_CHECK(bulk::min(world, s + 1) == 1, "min of local values");
      BULK_CHECK(bulk::sum(world, s + 1) == p * (p + 1) / 2,
                 "sum of local values");
      BULK_CHECK(bulk::product(world, 1) == 1, "product of local values");

      auto x = bulk::var<int>(world, s + 1);
      BULK_CHECK(bulk::max(x) == p, "max of var");
      BULK_CHECK(bulk::min(x) == 1, "min of var");
      BULK_CHECK(bulk::sum(x) == p * (p + 1) / 2, "sum of var");
      x = 1;
      BULK_CHECK(bulk::product(x) == 1, "product of var");

      bulk::coarray<int> xs(world, 2, 1);
      BULK_CHECK(bulk::product(xs) == 1, "product of coarray");
      xs[0] = 2 * s + 1;
      xs[1] = 2 * s + 2;
      BULK_CHECK(bulk::max(xs) == 2 * p, "max of coarray");
      BULK_CHECK(bulk::min(xs) == 1, "min of coarray");
      BULK_CHECK(bulk::sum(xs) == 2 * p * (2 * p + 1) / 2, "sum of coarray ");

      BULK_CHECK(
          bulk::sum(world, (int64_t)std::numeric_limits<int32_t>::max()) ==
              (int64_t)p * (int64_t)std::numeric_limits<int32_t>::max(),
          "sum of large values");
      BULK_CHECK(
          bulk::product(
              world,
              s == 0 ? (int64_t)std::numeric_limits<int32_t>::max() + 1 : 1) ==
              (int64_t)std::numeric_limits<int32_t>::max() + 1,
          "large product");
    }
  });
}
