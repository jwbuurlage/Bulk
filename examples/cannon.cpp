// Cannon's algorithm is a way to compute C = AB on a square grid of N x N
// processors. If all matrices M \in {A, B, C} are split up as follows:
//   _                          _
//  |  M_11 | M_12 | ... | M_1N  |
//  |  -----+------+-----+-----  |
//  |  M_21 | M_22 | ... | M_2N  |
//  |  -----+------+-----+-----  |
//  |  ...  | ...  | ... | ...
//  |  -----+------+-----+-----  |
//  |  M_N1 | M_N2 | ... | M_NN  |
//  |_                          _|
//
//  Then we can write for the block C_ij :
//    C_ij = \sum_{k = 1}^N A_ik B_kj
//
//  The idea of Cannon's algorithm is to let processor (s, t) compute block
//  C_st. If that processor starts with blocks A_(1 + s, 1 + (s + t) % N) and
//  B_(1 + (s + t) % N, t), then initially each processor has a unique block
//  assigned. After each iteration a processor sends its A-block to the
//  processor 'to the right' (i.e. at ((s + 1) % N, t)) and similarly sends its
//  B-block down. After N iterations, each processor has computed C_ij, so that
//  the result is available distributed over the processors.

#include <cassert>
#include <random>
#include <ranges>

#include "bulk/bulk.hpp"
#include "bulk/partitionings/partitioning.hpp"
#include "set_backend.hpp"

using T = float;

constexpr int kDimension = 1024;
constexpr int kGridDimension = 4;

auto rand01() {
  static std::uniform_real_distribution<T> distr(0.0, 1.0);
  static std::random_device device;
  static std::mt19937 engine(device());
  return distr(engine);
}

auto local_matrix_product(const bulk::block_partitioning<2, 2>& P,
                          bulk::coarray<T>& A, bulk::coarray<T>& B,
                          bulk::coarray<T>& C) {
  auto [m, n] = P.block_size().get();
  assert(m == n);

  // C_ij = Sum_k A_ik B_kj
  for (auto i = 0u; i < m; ++i) {
    for (auto j = 0u; j < m; ++j) {
      for (auto k = 0u; k < m; ++k) {
        C[bulk::util::flatten(P.block_size(), {i, j})] +=
            A[bulk::util::flatten(P.block_size(), {i, k})] *
            B[bulk::util::flatten(P.block_size(), {k, j})];
      }
    }
  }
}

int main() {
  environment env;

  // This example has to run with a fixed processor count.
  assert(env.available_processors() == kGridDimension * kGridDimension);

  env.spawn(env.available_processors(), [](bulk::world& world) {
    const bulk::index<2> processor_grid = {kGridDimension, kGridDimension};
    auto P = bulk::block_partitioning<2, 2>({kDimension, kDimension},
                                            processor_grid);

    const auto& [s1, s2] = P.multi_rank(world.rank()).get();
    const auto& [m, n] = P.block_size().get();
    assert(m == n);

    // Generate two (local) matrices with random [0, 1] entries.
    auto A = bulk::coarray<T>(world, m * n);
    auto B = bulk::coarray<T>(world, m * n);
    std::ranges::generate(A, rand01);
    std::ranges::generate(B, rand01);

    auto C = bulk::coarray<T>(world, m * n);

    const auto horizontal_neighbour =
        bulk::util::flatten(processor_grid, {(s1 + 1) % kGridDimension, s2});
    const auto vertical_neighbour =
        bulk::util::flatten(processor_grid, {(s1), (s2 + 1) % kGridDimension});

    for (int iteration = 0; iteration < kGridDimension; ++iteration) {
      world.log_once("Iteration: %i", iteration);
      local_matrix_product(P, A, B, C);

      A(horizontal_neighbour)
      [{0u, m * n}] = std::span(A.data(), A.size());

      B(vertical_neighbour)
      [{0u, m * n}] = std::span(B.data(), B.size());

      world.sync();
    }

    // C now contains the (local block of) the matrix product
    world.log_once("Finished computing C = AB.");
  });

  return 0;
}
