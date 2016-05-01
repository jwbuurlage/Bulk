#include <catch.hpp>

#include <bulk/bsp/bulk.hpp>
#include <bulk/util/log.hpp>

#include "bulk_test_common.hpp"

namespace bulk = bulk_bsp;

TEST_CASE("basic communication", "[init]") {
    SECTION("put") {
        auto center = bulk::center();

        center.spawn(center.available_processors(), [&center](int s, int p) {
            bulk::var<int> a;

            center.put(center.next_processor(), s, a);
            center.sync();

            BULK_CHECK_ONCE(a.value() == ((s + p - 1) % p));
        });
    }

    SECTION("get") {
        auto center = bulk::center();

        center.spawn(center.available_processors(), [&center](int s, int p) {
            bulk::var<int> a;
            a.value() = s;

            auto b = center.get(center.next_processor(), a);
            center.sync();

            BULK_CHECK_ONCE(b.value() == center.next_processor());
        });
    }
}
