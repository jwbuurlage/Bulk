#include <vector>

#include <catch.hpp>

#include <bulk/hub.hpp>
#include <bulk/variable.hpp>
#include <bulk/algorithm.hpp>
#include <bulk/bsp/bulk.hpp>
#include <bulk/util/log.hpp>

#include "bulk_test_common.hpp"

using provider = bulk::bsp::provider;

TEST_CASE("convenience functions", "[algorithm]") {
    SECTION("reduce") {
        auto hub = bulk::hub<provider>();

        hub.spawn(hub.available_processors(), [&hub](int s, int p) {
            auto result = bulk::create_var<int>(hub);
            result.value() = s;

            hub.sync();

            auto reduce_result =
                bulk::reduce(result, [](int& lhs, int rhs) { lhs += rhs; });

            int test_value = 0;
            for (int t = 0; t < p; ++t) {
                test_value += t;
            }

            BULK_CHECK_ONCE(reduce_result == test_value);
        });
    }
}
