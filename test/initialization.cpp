#include <catch.hpp>

#include <bulk/bulk.hpp>
#include <bulk/bsp/provider.hpp>
#include <bulk/util/log.hpp>

#include "bulk_test_common.hpp"


TEST_CASE("initializing bulk", "[init]") {
    SECTION("spawn") {
        auto env = bulk::environment<bulk::bsp::provider>();

        env.spawn(env.available_processors(), [](auto world, int s, int p) {
            BULK_CHECK_ONCE(s == world.processor_id());
            BULK_CHECK_ONCE(p == world.active_processors());
        });
    }

    SECTION("making vars") {
        auto env = bulk::environment<bulk::bsp::provider>();

        env.spawn(env.available_processors(), [](auto world, int, int) {
            // we can create a variable
            auto x = bulk::create_var<int>(world);
            x = 5;
            BULK_CHECK_ONCE(x.value() == 5);
            // we can create multiple variables in one step
            auto y = bulk::create_var<int>(world);
            // we can reassign variables
            x = bulk::create_var<int>(world);
            BULK_CHECK_ONCE(x.value() == 0);
        });
    }
}
