#include <catch.hpp>

#include <bulk/bsp/bulk.hpp>
#include <bulk/util/log.hpp>

#include "bulk_test_common.hpp"


TEST_CASE("initializing bulk", "[init]") {
    SECTION("spawn") {
        auto hub = bulk::bsp_hub();

        hub.spawn(hub.available_processors(),
                     [&hub](int s, int p) {
             BULK_CHECK_ONCE(s == hub.processor_id());
         });
    }
}
