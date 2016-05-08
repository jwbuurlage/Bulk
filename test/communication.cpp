#include <vector>
#include <numeric>

#include <catch.hpp>

#include <bulk/hub.hpp>
#include <bulk/variable.hpp>
#include <bulk/future.hpp>
#include <bulk/coarray.hpp>
#include <bulk/communication.hpp>
#include <bulk/bsp/bulk.hpp>
#include <bulk/util/log.hpp>

#include "bulk_test_common.hpp"

using provider = bulk::bsp::provider;

TEST_CASE("basic communication", "[communication]") {
    SECTION("put") {
        auto hub = bulk::hub<provider>();

        hub.spawn(hub.available_processors(), [&hub](int s, int p) {
            auto a = bulk::create_var<int>(hub);

            bulk::put(hub.next_processor(), s, a);
            hub.sync();

            BULK_CHECK_ONCE(a.value() == ((s + p - 1) % p));
        });
    }

    SECTION("multiple put") {
        auto hub = bulk::hub<provider>();

        hub.spawn(hub.available_processors(), [&hub](int s, int p) {
            int size = 5;
            std::vector<bulk::var<int, decltype(hub)>> xs;
            for (int i = 0; i < size; ++i)
                xs.push_back(bulk::create_var<int>(hub));

            for (int i = 0; i < size; ++i) {
                bulk::put(hub.next_processor(), s + i, xs[i]);
            }

            hub.sync();

            for (int i = 0; i < size; ++i) {
                BULK_CHECK_ONCE(xs[i].value() == ((s + p - 1) % p) + i);
            }
        });
    }

    SECTION("get") {
        auto hub = bulk::hub<provider>();

        hub.spawn(hub.available_processors(), [&hub](int s, int) {
            auto a = bulk::create_var<int>(hub);
            a.value() = s;

            auto b = bulk::get(hub.next_processor(), a);
            hub.sync();

            BULK_CHECK_ONCE(b.value() == hub.next_processor());
        });
    }

    SECTION("multiple get") {
        auto hub = bulk::hub<provider>();

        hub.spawn(hub.available_processors(), [&hub](int s, int) {
            auto x = bulk::create_var<int>(hub);
            x.value() = s;

            int size = 5;
            std::vector<bulk::future<int, decltype(hub)>> ys;
            for (int i = 0; i < size; ++i) {
                ys.push_back(bulk::get(hub.next_processor(), x));
            }

            hub.sync();

            for (auto& y : ys) {
                BULK_CHECK_ONCE(y.value() == hub.next_processor());
            }
        });
    }

    SECTION("message passing") {
        auto hub = bulk::hub<provider>();

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

    SECTION("coarrays") {
        auto hub = bulk::hub<provider>();

        hub.spawn(hub.available_processors(), [&hub](int s, int) {
            auto xs = bulk::create_coarray<int>(hub, 10);
            xs(hub.next_processor())[1] = s;

            hub.sync();

            BULK_CHECK_ONCE(xs[1] == hub.prev_processor());

            xs[3] = 2;

            BULK_CHECK_ONCE(xs[3] == 2);
        });
    }
}
