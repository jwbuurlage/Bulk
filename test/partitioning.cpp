#include <cmath>

#include <bulk/bulk.hpp>
#include "bulk_test_common.hpp"
#include "set_backend.hpp"

extern environment env;

void test_partitioning() {
    env.spawn(env.available_processors(), [](auto& world, int s, int p) {
        auto N = (int)sqrt(p);
        BULK_REQUIRE(N * N == p);

        BULK_SECTION("Cyclic partitioning to 1D") {
            auto part =
                bulk::cyclic_partitioning<2, 1>({p}, {5 * p * N, 5 * p * N});
            BULK_CHECK_ONCE(part.owner({p + 2, 3})[0] == 2,
                            "compute correctly the cyclic from 2 -> 1 dim");
            BULK_CHECK_ONCE(part.local_extent({s})[0] == 5 * N,
                            "compute correctly the extent in cyclic dim");
            BULK_CHECK_ONCE(part.local_extent({s})[1] == 5 * p * N,
                            "compute correctly the extent in non-cyclic dim");
            BULK_CHECK_ONCE(part.local_index({p + 2, 3})[0] == 1,
                            "compute correctly the local index");
        }

        BULK_SECTION("Cyclic partitioning") {
            auto part = bulk::cyclic_partitioning<2>({N, N}, {10 * N, 10 * N});
            BULK_CHECK_ONCE(part.owner({4, 3})[0] == 4 % N,
                            "compute correctly the cyclic owner");
            BULK_CHECK_ONCE(part.local_extent({s % N, s / N})[0] == 10,
                            "compute correctly the cyclic extent");
            BULK_CHECK_ONCE(part.local_index({4, 3})[0] == 4 / N,
                            "compute correctly the cyclic local index");
        }

        BULK_SECTION("Block partitioning") {
            auto part = bulk::block_partitioning<2>({N, N}, {10 * N, 10 * N});
            BULK_CHECK_ONCE(part.owner({2 * 10 + 3, 3})[0] == 2,
                            "compute correctly the block owner");
            BULK_CHECK_ONCE(part.local_extent({s % N, s / N})[0] == 10,
                            "compute correctly the block extent");
            BULK_CHECK_ONCE(part.local_index({3, 12})[1] == 2,
                            "compute correctly the block index");
            BULK_CHECK_ONCE(part.origin(0)[0] == 0,
                            "compute correctly the block origin (0)");
            BULK_CHECK_ONCE(part.origin(1)[0] == 10,
                            "compute correctly the block origin (1)");
            BULK_CHECK_ONCE(part.origin(2)[1] == 10,
                            "compute correctly the block origin (2)");
        }

        BULK_SECTION("Binary-split-partitioning") {
            using dir = bulk::binary_tree<bulk::split>::dir;
            auto tree = bulk::binary_tree<bulk::split>(bulk::split{0, 5});
            auto root = tree.root.get();
            tree.add(root, dir::left, bulk::split{1, 5});
            tree.add(root, dir::right, bulk::split{1, 5});

            auto part =
                bulk::binary_partitioning<2>(4, {10, 10}, std::move(tree));

            BULK_CHECK_ONCE((part.local_extent({0}) == std::array<int, 2>{5, 5}),
                            "extent of bspart are correct");

            BULK_CHECK_ONCE((part.owner({1, 1}) == part.owner({2, 2})),
                            "assign correct owner bspart (1)");

            BULK_CHECK_ONCE((part.owner({1, 1}) != part.owner({6, 7})),
                            "assign correct owner bspart (2)");

            BULK_CHECK_ONCE((part.origin(1) == std::array<int, 2>{5, 0}),
                            "assign correct origin bspart (1)");
            BULK_CHECK_ONCE((part.origin(2) == std::array<int, 2>{0, 5}),
                            "assign correct origin bspart (2)");
            BULK_CHECK_ONCE((part.origin(3) == std::array<int, 2>{5, 5}),
                            "assign correct origin bspart (3)");

            BULK_CHECK_ONCE((part.local_index({6, 6}) == std::array<int, 2>{1, 1}),
                            "assign correct origin bspart");

        }

        BULK_SECTION("Partitioned array") {
            auto part = bulk::cyclic_partitioning<2>({N, N}, {200, 200});
            auto xs =
                bulk::partitioned_array<int, 2>(world, part);

            xs.local(0, 0) = s;
            xs.local(1, 1) = s + 1;

            auto glob = xs.global(1, 1).get();
            world.sync();

            BULK_CHECK_ONCE(glob.value() == N + 1, "obtain remote value");
            BULK_CHECK_ONCE(xs.local(1, 1) == s + 1, "obtain local value");

            xs.global(1, 1) = 1234;
            world.sync();

            glob = xs.global(1, 1).get();
            world.sync();

            BULK_CHECK_ONCE(glob.value() == 1234, "put remote value");
        }
    });
}
