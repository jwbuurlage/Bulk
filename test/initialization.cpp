#include <catch.hpp>

#include <bulk/bsp/bulk.hpp>
#include <bulk/util/log.hpp>

#include "bulk_test_common.hpp"

namespace bulk = bulk_bsp;


TEST_CASE("initializing bulk", "[init]") {
    SECTION("spawn") {
        auto center = bulk::center();

        center.spawn(center.available_processors(),
                     [&center](int s, int p) {
             BULK_CHECK_ONCE(s == center.processor_id());
         });
    }
}
