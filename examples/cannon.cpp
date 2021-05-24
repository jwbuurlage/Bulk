#include "bulk/bulk.hpp"

#include <cassert>
#include <random>
#include <ranges>

#include "bulk/partitionings/partitioning.hpp"
#include "set_backend.hpp"

using T = float;

constexpr int kDimension = 1024;
constexpr int kGridDimension = 4;

namespace {

auto rand01() {
    static std::uniform_real_distribution<T> distr(0.0, 1.0);
    static std::random_device device;
    static std::mt19937 engine(device());
    return distr(engine);
}

auto local_matrix_product(const bulk::block_partitioning<2, 2>& P,
                          bulk::coarray<T>& A,
                          bulk::coarray<T>& B,
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

} // namespace

int main() {
    environment env;

    // This example has to run with a fixed processor count.
    assert(env.available_processors() == kGridDimension * kGridDimension);

    env.spawn(env.available_processors(), [](bulk::world& world) {
        const bulk::index<2> processor_grid = {kGridDimension, kGridDimension};
        auto P = bulk::block_partitioning<2, 2>({kDimension, kDimension}, processor_grid);

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
            detail::local_matrix_product(P, A, B, C);

            A(horizontal_neighbour)
            [{0u, m * n}] = bulk::span(A.data(), A.size());

            B(vertical_neighbour)
            [{0u, m * n}] = bulk::span(B.data(), B.size());

            world.sync();
        }

        // C now contains the (local block of) the matrix product
        world.log("%f", C[0]);
    });

    return 0;
}
