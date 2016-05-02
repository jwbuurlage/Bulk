#include <vector>

#include <catch.hpp>

#include <bulk/bsp/bulk.hpp>
#include <bulk/util/log.hpp>

#include "bulk_test_common.hpp"

namespace bulk = bulk_bsp;


TEST_CASE("basic communication", "[communication]") {
    SECTION("put") {
        auto center = bulk::center();

        center.spawn(center.available_processors(), [&center](int s, int p) {
            bulk::var<int> a;

            center.put(center.next_processor(), s, a);
            center.sync();

            BULK_CHECK_ONCE(a.value() == ((s + p - 1) % p));
        });
    }

    SECTION("multiple put") {
        auto center = bulk::center();

        center.spawn(center.available_processors(), [&center](int s, int p) {
            int size = 5;
            std::vector<bulk::var<int>> xs(size);

            for (int i = 0; i < size; ++i) {
                center.put(center.next_processor(), s + i, xs[i]);
            }
            center.sync();

            for (int i = 0; i < size; ++i) {
                BULK_CHECK_ONCE(xs[i].value() == ((s + p - 1) % p) + i);
            }
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

    SECTION("multiple get") {
        auto center = bulk::center();

        center.spawn(center.available_processors(), [&center](int s, int p) {
            bulk::var<int> x;
            x.value() = s;

            int size = 5;
            std::vector<bulk::future<int>> ys(5);
            for (auto& y : ys) {
                y = center.get(center.next_processor(), x);
            }

            center.sync();

            for (auto& y : ys) {
                BULK_CHECK_ONCE(y.value() == center.next_processor());
            }
        });
    }

    SECTION("message passing") {
        auto center = bulk::center();

        center.spawn(center.available_processors(), [&center](int s, int p) {
            for (int t = 0; t < p; ++t) {
                center.send<int, int>(t, s, s);
            }

            center.sync();

            std::vector<int> contents;
            for (auto message : center.messages<int, int>()) {
                contents.push_back(message.content);
            }
            std::sort(contents.begin(), contents.end());

            std::vector<int> compare_result(p);
            std::iota(compare_result.begin(), compare_result.end(), 0);

            BULK_CHECK_ONCE(compare_result == contents);
        });
    }
}
