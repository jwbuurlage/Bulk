#include <chrono>
#include <thread>

#include "bulk_test_common.hpp"
#include "set_backend.hpp"
#include <bulk/bulk.hpp>

extern environment env;

void test_communication() {
    env.spawn(env.available_processors(), [](auto& world) {
        int s = world.rank();
        int p = world.active_processors();

        BULK_SKIP_SECTION_IF("Communication", p <= 1);

        BULK_SECTION("Put") {
            // test `put` to single variable
            bulk::var<int> a(world, 3);

            BULK_CHECK(a.value() == 3, "correct initial value for variable");

            bulk::put(world.next_rank(), s, a);
            world.sync();

            BULK_CHECK(a.value() == ((s + p - 1) % p),
                       "receive correct value after putting");
        }

        BULK_SECTION("Broadcast") {
            // test `put` to single variable
            bulk::var<int> a(world, 3);

            if (world.rank() == 1) {
                a.broadcast(2);
            }
            world.sync();

            BULK_CHECK(a.value() == 2,
                       "receive correct value after broadcasting");
        }

        BULK_SECTION("Put and get delayed") {
            using namespace std::chrono_literals;

            // test `put` to single variable
            bulk::var<int> a(world, s);

            bulk::put(world.next_rank(), s, a);

            // sleep
            std::this_thread::sleep_for(20ms);

            BULK_CHECK(a.value() == s, "local copy untouched before sync");

            world.sync();

            BULK_CHECK(a.value() == world.prev_rank(),
                       "receive data after sync");

            a.value() = 42;

            world.sync();

            auto b = bulk::get(world.next_rank(), a);

            std::this_thread::sleep_for(20ms);

            a.value() = 45;

            world.sync();

            BULK_CHECK(b.value() == 45, "receive correct value after get");
        }

        BULK_SECTION("Sugarized put") {
            // test `put` to single variable
            bulk::var<int> a(world);

            a(world.next_rank()) = s;
            world.sync();

            BULK_CHECK(a.value() == ((s + p - 1) % p),
                       "receive correct value after sugarized putting");
        }

        BULK_SECTION("Put to self") {
            bulk::var<int> a(world);

            bulk::put(s, s, a);
            world.sync();

            BULK_CHECK(a.value() == s,
                       "receive correct value after putting to self");
        }

        BULK_SECTION("Get and put play nice together") {
            bulk::var<int> a(world, 15);

            a(0) = 5;
            auto b = a(0).get();

            world.sync();

            BULK_CHECK(b == 15, "gets before puts");
        }

        BULK_SECTION("Get from self") {
            bulk::var<int> a(world);
            a = s;
            auto b = bulk::get(s, a);
            world.sync();

            BULK_CHECK(b.value() == s,
                       "receive correct value after getting from self");
        }

        BULK_SECTION("Put non-int") {
            // test `put` float to single variable
            bulk::var<float> a(world, 5.0f);

            bulk::put(world.next_rank(), 1.0f, a);
            world.sync();

            BULK_CHECK(a.value() == 1.0f,
                       "receive correct value after putting float");
        }

        BULK_SECTION("Put custom struct") {
            struct custom_struct {
                int x;
                float y;
            };
            bulk::var<custom_struct> a(world, {4, 8.0f});

            a(world.next_rank()) = {3, 2.0f};
            world.sync();

            BULK_CHECK(a.value().x == 3 && a.value().y == 2.0f,
                       "receive correct value after putting custom struct");
        }

        BULK_SECTION("Put a string") {
            bulk::var<std::string> a(world);

            a(world.next_rank()) = "test";
            world.sync();

            BULK_CHECK(a.value() == "test", "can put a string ");
        }

        BULK_SECTION("Get a string") {
            bulk::var<std::string> a(world, "test" + std::to_string(s));

            auto b = a(world.next_rank()).get();
            world.sync();

            BULK_CHECK(b.value() == "test" + std::to_string(world.next_rank()),
                       "can get a string");
        }

        BULK_SECTION("Put multiple") {
            int size = 5;

            // test `put` to multiple variables
            std::vector<bulk::var<int>> xs;
            for (int i = 0; i < size; ++i) {
                xs.emplace_back(world);
            }

            for (int i = 0; i < size; ++i) {
                bulk::put(world.next_rank(), s + i, xs[i]);
            }

            world.sync();

            for (int i = 0; i < size; ++i) {
                BULK_CHECK(xs[i].value() == ((s + p - 1) % p) + i,
                           "receive correct value after multiple puts to "
                           "array of variables");
            }
        }

        BULK_SECTION("Put unequal") {
            int size = 5;

            // test `put` to multiple variables
            std::vector<bulk::var<int>> xs;
            for (int i = 0; i < size; ++i)
                xs.emplace_back(world);

            if (s == 0)
                for (int i = 1; i < p; ++i) {
                    for (int j = 0; j < size; ++j) {
                        bulk::put(i, i, xs[j]);
                    }
                }

            world.sync();

            BULK_CHECK(xs[2].value() == s,
                       "receive correct value after heterogeneous puts");

            bulk::future<int> a(world);
            if (s == 0)
                a = bulk::get(p - 1, xs[size - 1]);

            world.sync();

            BULK_CHECK_ONCE(a.value() == p - 1,
                            "receive correct value after getting");
        }

        BULK_SECTION("Get") {
            bulk::var<int> b(world);
            b.value() = s;
            world.sync();

            auto c = bulk::get(world.next_rank(), b);
            world.sync();

            BULK_CHECK(c.value() == world.next_rank(),
                       "receive correct value after getting");
        }

        BULK_SECTION("Sugarized get") {
            bulk::var<int> b(world);
            b.value() = s;
            world.sync();

            auto c = b(world.next_rank()).get();
            world.sync();

            BULK_CHECK(c.value() == world.next_rank(),
                       "receive correct value after sugarized getting");
        }

        BULK_SECTION("Get multiple") {
            int size = 5;
            bulk::var<int> x(world);
            x.value() = s;

            world.sync();

            std::vector<bulk::future<int>> ys;
            for (int i = 0; i < size; ++i) {
                ys.push_back(bulk::get(world.next_rank(), x));
            }

            world.sync();

            for (auto& y : ys) {
                BULK_CHECK(y.value() == world.next_rank(),
                           "receive correct value after getting multiple");
            }
        }

        BULK_SECTION("Put array") {
            std::vector<int> test(10);
            std::iota(test.begin(), test.end(), 1);

            bulk::coarray<int> xs(world, 10, 5);
            xs.put(world.next_rank(), test.begin(), test.end());
            world.sync();

            BULK_CHECK(xs[5] == 6, "put iterator range");
        }

        BULK_SECTION("Coarray") {
            bulk::coarray<int> zs(world, 10);

            BULK_CHECK(zs.size() == 10, "can obtain the size of a coarray");
            BULK_CHECK(zs.empty() == false, "can check for emptyness");

            zs(world.next_rank())[1] = s;

            world.sync();

            BULK_CHECK(zs[1] == world.prev_rank(),
                       "putting to remote coarray image gives correct result");

            zs[3] = 2;

            BULK_CHECK(zs[3] == 2,
                       "writing to local coarray gives correct result");

            auto a = zs(0)[1].get();
            world.sync();

            BULK_CHECK(a.value() == p - 1,
                       "getting from coarray gives correct result");

            zs(s)[3] = 1234;
            world.sync();
            BULK_CHECK(zs[3] == 1234, "put to self with coarray");
        }

        BULK_SECTION("Coarray iteration") {
            auto xs = bulk::gather_all(world, s);
            int t = 0;
            for (auto x : xs) {
                BULK_CHECK(x == t++, "gather operation succeeded");
            }
        }

        BULK_SECTION("Coarray slicing") {
            bulk::coarray<int> zs(world, 10, 0);
            zs(world.next_rank())[{2, 5}] = s;
            zs(world.next_rank())[{0, 2}] = {s - 1, s - 2};

            world.sync();

            for (int i = 2; i < 5; ++i) {
                BULK_CHECK(zs[i] == world.prev_rank(),
                           "setting slice to constant");
            }
            BULK_CHECK(zs[5] == 0, "outside the slice values are untouched");
            BULK_CHECK(zs[0] == world.prev_rank() - 1,
                       "individual slice setting");
            BULK_CHECK(zs[1] == world.prev_rank() - 2,
                       "individual slice setting");
        }

        BULK_SECTION("All-to-all coarray slicing") {
            auto local = std::vector<int>(p);
            std::iota(local.begin(), local.end(), s * p);

            auto samples = bulk::coarray<int>(world, p * p);
            for (int t = 0; t < p; ++t) {
                samples(t)[{s * p, (s + 1) * p}] = local;
            }
            world.sync();

            bool flag = true;
            for (int i = 0; i < p * p; ++i) {
                if (samples[i] != i) {
                    flag = false;
                    break;
                }
            }
            BULK_CHECK(flag, "all-to-all slicing with equal blocks");

            auto local_size = 1024;
            auto local_large = std::vector<int>(local_size);
            std::iota(local_large.begin(), local_large.end(), s * local_size);

            auto large_samples = bulk::coarray<int>(world, p * local_size);
            for (int t = 0; t < p; ++t) {
                large_samples(t)[{s * local_size,
                                  s * local_size + 1 + (local_size / p) * s}] =
                    local_large;
            }
            world.sync();

            bool large_flag = true;
            for (int i = 0; i < p; ++i) {
                if (large_samples[i * local_size] != i * local_size) {
                    large_flag = false;
                    break;
                }
            }
            BULK_CHECK(large_flag, "all-to-all slicing with unequal blocks");
        }

        BULK_SECTION("Get slice") {
            bulk::coarray<int> zs(world, 10, 0);
            std::iota(zs.begin(), zs.end(), 0);

            auto xs = zs((s + 2) % p)[{2, 6}].get();
            world.sync();

            bool flag = false;
            for (int i = 2; i < 6; ++i) {
                if (xs[i] != i) {
                    flag = true;
                }
            }
            BULK_CHECK(flag, "getting slices");
        }

        BULK_SECTION("Single message passing") {
            bulk::queue<int, int> q(world);
            q(world.next_rank()).send(123, 1337);
            world.sync();
            for (auto [tag, content] : q) {
                BULK_CHECK(tag == 123 && content == 1337,
                           "message passed succesfully");
            }
        }

        BULK_SECTION("Message to self") {
            bulk::queue<int, int> q(world);

            q(world.rank()).send(123, 1337);
            world.sync();
            int tag = 0;
            int content = 0;
            for (auto [x, y] : q) {
                tag = x;
                content = y;
            }
            BULK_CHECK(tag == 123 && content == 1337,
                       "message passed succesfully");
        }

        BULK_SECTION("Multiple messages to self") {
            auto q = bulk::queue<int, int>(world);

            q(world.rank()).send(123, 1337);
            world.sync();
            int tag = 0;
            int content = 0;
            for (auto [x, y] : q) {
                tag = x;
                content = y;
            }
            BULK_CHECK(tag == 123 && content == 1337,
                       "message passed succesfully");
        }

        BULK_SECTION("Multiple message passing") {
            std::vector<int> contents = {1337, 12345, 1230519, 5, 8};

            bulk::queue<int, int> q(world);
            for (size_t i = 0; i < contents.size(); ++i) {
                q(world.next_rank()).send(s, contents[i]);
            }

            world.sync();

            int k = 0;
            BULK_CHECK(!q.empty(), "multiple messages arrived");
            for (auto [tag, content] : q) {
                BULK_CHECK(tag == world.prev_rank() && content == contents[k++],
                           "multiple messages passed succesfully");
            }
        }

        BULK_SECTION("Multiple queue and types message passing") {
            std::vector<int> contents = {1337, 12345, 1230519, 5, 8};
            std::vector<float> contents2 = {1.0f, 2.0f, 3.0f, 4.0f};

            bulk::queue<int> q(world);
            bulk::queue<int, float> q2(world);

            for (size_t i = 0; i < contents.size(); ++i) {
                q(world.next_rank()).send(contents[i]);
            }
            for (size_t i = 0; i < contents2.size(); ++i) {
                q2(world.next_rank()).send(s, contents2[i]);
            }

            world.sync();

            int k = 0;
            BULK_CHECK_ONCE(!q.empty() && !q2.empty(), "queues are non-empty");
            for (auto& msg : q) {
                BULK_CHECK_ONCE(msg == contents[k++],
                                "received correct result on q");
            }

            int l = 0;
            BULK_CHECK_ONCE(q.size() == contents.size(),
                            "first queue correct number of messages");

            BULK_CHECK_ONCE(q2.size() == contents2.size(),
                            "second queue correct number of messages");

            for (auto [tag, content] : q2) {
                BULK_CHECK(tag == world.prev_rank() &&
                               content == contents2[l++],
                           "received correct result on q2");
            }

            world.sync();

            BULK_CHECK(q.empty(), "first queue gets emptied");
            BULK_CHECK(q2.empty(), "second queue gets emptied");
        }

        BULK_SECTION("Messages with arrays") {
            auto q = bulk::queue<int[]>(world);
            q(world.next_rank()).send({1, 2, 3, 4});
            world.sync();
            BULK_CHECK(q.size() == 1, "send many is one message");
            for (auto msg : q) {
                BULK_CHECK((msg == std::vector<int>{1, 2, 3, 4}),
                           "send many correct content");
            }
        }

        BULK_SECTION("Messages with multiple arrays") {
            auto q = bulk::queue<int[], float[]>(world);
            q(world.next_rank()).send({1, 2, 3, 4}, {1.0f, 2.0f});
            world.sync();
            BULK_CHECK(q.size() == 1, "send many is one message");
            for (auto [xs, fs] : q) {
                BULK_CHECK((xs == std::vector<int>{1, 2, 3, 4}),
                           "send many first content");
                BULK_CHECK((fs == std::vector<float>{1.0f, 2.0f}),
                           "send many second content");
            }
        }

        BULK_SECTION("Messages with arrays and normal") {
            auto q = bulk::queue<int[], int>(world);
            q(world.next_rank()).send({1, 2, 3, 4}, 1);
            q(world.next_rank()).send({2, 3, 4, 5}, 2);
            world.sync();
            BULK_CHECK(q.size() == 2, "send many with two messages");
            for (auto [xs, x] : q) {
                BULK_CHECK(x == 1 || x == 2, "right 'tag'");
                if (x == 1) {
                    BULK_CHECK((xs == std::vector<int>{1, 2, 3, 4}),
                               "send many correct content #1");
                } else {
                    BULK_CHECK((xs == std::vector<int>{2, 3, 4, 5}),
                               "send many correct content #2");
                }
            }
        }

        BULK_SECTION("Messages with strings") {
            auto q = bulk::queue<int, std::string>(world);
            q(world.next_rank()).send(5, "string");
            world.sync();
            BULK_CHECK(q.size() == 1, "string message received");
            for (auto [size, str] : q) {
                BULK_CHECK(str == "string" && size == 5, "can send string");
            }
        }
    });
}
