#include <bulk/bulk.hpp>
#include <cmath>

#include "bulk_test_common.hpp"
#include "set_backend.hpp"

namespace bulk {
using namespace experimental;
}

extern environment env;

void test_partitioning() {
  auto N = (int)sqrt(env.available_processors());
  env.spawn(N * N, [](auto& world) {
    int s = world.rank();
    // Store the number of processors as 'size_t' for compatibility with
    // partitioning indices.
    size_t p = world.active_processors();

    BULK_SECTION("Index manipulations") {
      bulk::index<2> idx{2, 3};
      BULK_CHECK(idx[1] == 3, "can construct with initializer list");
    }

    auto N = static_cast<size_t>(sqrt(p));
    BULK_SKIP_SECTION_IF("Partitionings", N * N != p);
    BULK_SKIP_SECTION_IF("Partitionings", p <= 1);

    BULK_SECTION("Cyclic partitioning") {
      auto part = bulk::cyclic_partitioning<2, 2>({10 * N, 10 * N}, {N, N});
      BULK_CHECK(part.multi_owner({4, 3})[0] == 4 % N,
                 "compute correctly the cyclic owner");
      BULK_CHECK(part.local_size({s % N, s / N})[0] == 10,
                 "compute correctly the cyclic size");
      BULK_CHECK(part.local({4, 3})[0] == 4 / N,
                 "compute correctly the cyclic local index");
      BULK_CHECK(part.global({1, 1}, {s % N, s / N})[0] == N + (s % N),
                 "compute correctly the cyclic global index");
    }

    BULK_SECTION("Cyclic partitioning to 1D") {
      auto part = bulk::cyclic_partitioning<2, 1>({5 * p * N, 5 * p * N}, p);
      BULK_CHECK(part.owner({p + 2, 3}) == 2,
                 "compute correctly the cyclic from 2 -> 1 dim");
      BULK_CHECK(part.local_size(s)[0] == 5 * N,
                 "compute correctly the extent in cyclic dim");
      BULK_CHECK(part.local_size(s)[1] == 5 * p * N,
                 "compute correctly the extent in non-cyclic dim");
      BULK_CHECK(part.local({p + 2, 3})[0] == 1,
                 "compute correctly the local index");
    }

    BULK_SECTION("Block partitioning") {
      auto part = bulk::block_partitioning<2, 2>({10 * N, 10 * N}, {N, N});
      BULK_CHECK(part.multi_owner({2 * 10 + 3, 3})[0] == 2,
                 "compute correctly the block owner");
      BULK_CHECK(part.local_size({s % N, s / N})[0] == 10,
                 "compute correctly the block extent");
      BULK_CHECK(part.local({3, 12})[1] == 2,
                 "compute correctly the block index");
      BULK_CHECK(part.origin(0)[0] == 0,
                 "compute correctly the block origin (0)");
      BULK_CHECK(part.origin(1)[0] == 10,
                 "compute correctly the block origin (1)");
      BULK_CHECK(part.origin(p - 1)[0] == 10 * (N - 1),
                 "compute correctly the block origin (2)");
      BULK_CHECK(part.origin(p - 1)[1] == 10 * (N - 1),
                 "compute correctly the block origin (3)");
    }

    BULK_SECTION("Block partitioning custom axes") {
      // construct a block partitioning only in the 2nd axis
      auto part = bulk::block_partitioning<2, 1>({10 * p, 10 * p}, p, 1);

      BULK_CHECK(part.owner({0, 13}) == 1, "compute correctly the block owner");
      BULK_CHECK(part.local_size(1)[0] == 10 * p,
                 "compute correctly the block size [0]");
      BULK_CHECK(part.local_size(1)[1] == 10,
                 "compute correctly the block size [1]");
      BULK_CHECK(part.local({3, 12})[1] == 2,
                 "compute correctly the block index");
      BULK_CHECK(part.origin(0)[1] == 0,
                 "compute correctly the block origin (0)[1]");
      BULK_CHECK(part.origin(1)[1] == 10,
                 "compute correctly the block origin (1)[1]");
      BULK_CHECK(part.origin(2)[1] == 20,
                 "compute correctly the block origin (2)[1]");
      BULK_CHECK(part.origin(0)[0] == 0,
                 "compute correctly the block origin (0)[0]");
      BULK_CHECK(part.origin(1)[0] == 0,
                 "compute correctly the block origin (1)[0]");
    }

    BULK_SECTION("Binary-split-partitioning") {
      using dir = bulk::util::binary_tree<bulk::util::split>::dir;
      auto tree =
          bulk::util::binary_tree<bulk::util::split>(bulk::util::split{0, 4});
      auto root = tree.root.get();
      tree.add(root, dir::left, bulk::util::split{1, 4});
      tree.add(root, dir::right, bulk::util::split{1, 4});

      auto part = bulk::tree_partitioning<2>({10, 10}, 4, std::move(tree));

      BULK_CHECK((part.local_size(1) == bulk::index<2>{5, 5}),
                 "extent of bspart are correct");

      BULK_CHECK((part.owner({1, 1}) == part.owner({2, 2})),
                 "assign correct owner bspart (1)");

      BULK_CHECK((part.owner({1, 1}) != part.owner({6, 7})),
                 "assign correct owner bspart (2)");

      BULK_CHECK((part.origin(1) == bulk::index<2>{5, 0}),
                 "assign correct origin bspart (1)");
      BULK_CHECK((part.origin(2) == bulk::index<2>{0, 5}),
                 "assign correct origin bspart (2)");
      BULK_CHECK((part.origin(3) == bulk::index<2>{5, 5}),
                 "assign correct origin bspart (3)");

      BULK_CHECK((part.local({6, 6}) == bulk::index<2>{1, 1}),
                 "local computation bspart");
    }

    BULK_SECTION("Partitioned array") {
      auto part = bulk::cyclic_partitioning<2>({200, 200}, {N, N});
      auto xs = bulk::partitioned_array<int, 2, 2>(world, part);

      xs.local({0, 0}) = s;
      xs.local({1, 1}) = s + 1;

      auto glob = xs.global({1, 1}).get();
      world.sync();

      BULK_CHECK(glob.value() == static_cast<int>(N) + 1,
                 "obtain remote value");
      BULK_CHECK(xs.local({1, 1}) == s + 1, "obtain local value");

      xs.global({1, 1}) = 1234;
      world.sync();

      glob = xs.global({1, 1}).get();
      world.sync();

      BULK_CHECK(glob.value() == 1234, "put remote value");
    }

    BULK_SECTION("Irregular block partitioning") {
      auto part = bulk::block_partitioning<1>(5, 4);
      BULK_CHECK(part.local_count(0) == 2, "large block comes first");
      BULK_CHECK(part.local_count(1) == 1, "small block comes after");
      BULK_CHECK(part.local_count(3) == 1, "last block is small");
      BULK_CHECK(part.owner(0) == 0, "computes owner for 0 correctly")
      BULK_CHECK(part.owner(1) == 0, "computes owner for 1 correctly")
      BULK_CHECK(part.owner(3) == 2, "computes owner for 3 correctly")
      BULK_CHECK(part.owner(4) == 3, "computes owner for 4 correctly")

      BULK_CHECK(part.local(1) == 1, "local: first processor has two elements");
      BULK_CHECK(part.local(2) == 0,
                 "local: other processors have one element (2)");
      BULK_CHECK(part.local(3) == 0,
                 "local: other processors have one element (3)");
      BULK_CHECK(part.local(4) == 0,
                 "local: other processors have one element (4)");

      BULK_CHECK(part.global(1, 0) == 1,
                 "global: first processor has two elements");
      BULK_CHECK(part.global(0, 1) == 2,
                 "global: other processors have one element (2)");
      BULK_CHECK(part.global(0, 2) == 3,
                 "global: other processors have one element (3)");
      BULK_CHECK(part.global(0, 3) == 4,
                 "global: other processors have one element (4)");
    }
  });
}
