#include <vector>

#include <catch.hpp>

#include <bulk/bsp/bulk.hpp>
#include <bulk/util/log.hpp>

#include "bulk_test_common.hpp"


TEST_CASE("basic communication", "[communication]") {
    SECTION("put") {
        auto hub = bulk::bsp_hub();

        hub.spawn(hub.available_processors(), [&hub](int s, int p) {
            auto a = hub.create_var<int>();

            hub.put(hub.next_processor(), s, a);
            hub.sync();

            BULK_CHECK_ONCE(a.value() == ((s + p - 1) % p));
        });
    }

    SECTION("multiple put") {
        auto hub = bulk::bsp_hub();

        hub.spawn(hub.available_processors(), [&hub](int s, int p) {
            int size = 5;
            std::vector<bulk::bsp_hub::var_type<int>> xs(size);

            for (int i = 0; i < size; ++i) {
                hub.put(hub.next_processor(), s + i, xs[i]);
            }
            hub.sync();

            for (int i = 0; i < size; ++i) {
                BULK_CHECK_ONCE(xs[i].value() == ((s + p - 1) % p) + i);
            }
        });
    }

    SECTION("get") {
        auto hub = bulk::bsp_hub();

        hub.spawn(hub.available_processors(), [&hub](int s, int) {
            auto a = hub.create_var<int>();
            a.value() = s;

            auto b = hub.get(hub.next_processor(), a);
            hub.sync();

            BULK_CHECK_ONCE(b.value() == hub.next_processor());
        });
    }

    SECTION("multiple get") {
        auto hub = bulk::bsp_hub();

        hub.spawn(hub.available_processors(), [&hub](int s, int) {
            auto x = hub.create_var<int>();
            x.value() = s;

            int size = 5;
            std::vector<bulk::bsp_hub::future_type<int>> ys(5);
            for (auto& y : ys) {
                y = hub.get(hub.next_processor(), x);
            }

            hub.sync();

            for (auto& y : ys) {
                BULK_CHECK_ONCE(y.value() == hub.next_processor());
            }
        });
    }

    SECTION("message passing") {
        auto hub = bulk::bsp_hub();

        hub.spawn(hub.available_processors(), [&hub](int s, int p) {
            for (int t = 0; t < p; ++t) {
                hub.send<int, int>(t, s, s);
            }

            hub.sync();

            std::vector<int> contents;
            for (auto message : hub.messages<int, int>()) {
                contents.push_back(message.content);
            }
            std::sort(contents.begin(), contents.end());

            std::vector<int> compare_result(p);
            std::iota(compare_result.begin(), compare_result.end(), 0);

            BULK_CHECK_ONCE(compare_result == contents);
        });
    }
}
