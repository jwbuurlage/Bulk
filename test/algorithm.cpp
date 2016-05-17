#include <vector>

#include <catch.hpp>

#include <bulk/bulk.hpp>
#include <bulk/bsp/provider.hpp>
#include <bulk/util/log.hpp>

#include "bulk_test_common.hpp"

using provider = bulk::bsp::provider;

TEST_CASE("convenience functions", "[algorithm]") {
    SECTION("foldl") {
        auto env = bulk::environment<provider>();

        env.spawn(env.available_processors(), [](auto world, int s, int p) {
            auto result = bulk::create_var<int>(world);
            result.value() = s;

            world.sync();

            auto reduce_result =
                bulk::foldl(result, [](int& lhs, int rhs) { lhs += rhs; });

            int test_value = 0;
            for (int t = 0; t < p; ++t) {
                test_value += t;
            }

            BULK_CHECK_ONCE(reduce_result == test_value);
        });
    }
}
