#pragma once

#include <iomanip>

#include "bulk/bulk.hpp"

namespace psc {

template <typename T>
class matrix {
  public:
    matrix(bulk::world& world, bulk::multi_partitioning<2, 2>& partitioning)
        : world_(world), partitioning_(partitioning),
          data_(world_, partitioning.local_count(world_.rank()), (T)1) {
        multi_id_ =
            bulk::util::unflatten<2>(partitioning_.grid(), world_.rank());
    }

    T& at(bulk::index_type<2> index) {
        return data_[bulk::util::flatten<2>(partitioning_.local_size(multi_id_),
                                            index)];
    }

    auto& partitioning() { return partitioning_; };
    auto& world() { return world_; };

    auto rows() { return partitioning_.global_size()[0]; }
    auto cols() { return partitioning_.global_size()[1]; }

  private:
    bulk::index_type<2> multi_id_;
    bulk::world& world_;
    bulk::multi_partitioning<2, 2>& partitioning_;
    bulk::coarray<T> data_;
};

template <typename T>
void lu(matrix<T>& mat) {
    auto& world = mat.world();
    auto& phi = mat.partitioning();
    auto n = mat.rows();
    auto [s, t] = bulk::util::unflatten<2>(phi.grid(), world.rank());

    auto pivot = bulk::var<T>(world, -1);
    auto row_buffer = bulk::coarray<T>(world, n);
    auto col_buffer = bulk::coarray<T>(world, n);

    for (int k = 0; k < n; ++k) {
        world.log("stage: %i", k);

        // (0)
        // Find maximum local element in column k
        // (1)
        // Communicate maximum element with local processor column
        // (2)
        // Find global maximum
        // (3)
        // Communicate global maximum with local processor row
        // (4)
        // Communicate permutation update
        // (5)
        // Update local permutation vector
        // (6)
        // Communicate row k and r
        // (7)
        // Locally swap row k and r
        // (8)
        if (phi.owner({k, k}) == world.rank()) {
            for (auto u = 0; u < phi.grid()[0]; ++u) {
                pivot(bulk::util::flatten<2>(phi.grid(), {u, t})) =
                    mat.at(phi.global_to_local({k, k}));
            }
        }
        world.sync();

        // (9)
        auto j = phi.global_to_local({0, k})[1];
        // FIXME this is nicer if we have explicit Cartesian partitioning
        // support
        if (phi.grid_owner({0, k})[1] == t) {
            // local row index, local col size, loop
            // FIXME how to identify the first q such that global(q) > k
            for (auto i = 0; i < phi.local_size({s, t})[0]; ++i) {
                mat.at({i, j}) /= pivot;
            }
        }

        // (10)
        // Horizontal
        if (phi.grid_owner({0, k})[1] == t) {
            for (auto v = 0; v < phi.grid()[1]; ++v) {
                auto target_rank = bulk::util::flatten<2>(phi.grid(), {s, v});
                for (auto i = 0; i < phi.local_size({s, t})[0]; ++i) {
                    auto global_row = phi.local_to_global({i, 0}, {s, t})[0];
                    row_buffer(target_rank)[global_row] = mat.at({i, j});
                }
            }
        }
        // Vertical
        auto q = phi.global_to_local({k, 0})[1];
        if (phi.grid_owner({k, 0})[0] == s) {
            for (auto u = 0; u < phi.grid()[0]; ++u) {
                auto target_rank = bulk::util::flatten<2>(phi.grid(), {u, t});
                for (auto r = 0; r < phi.local_size({s, t})[1]; ++r) {
                    auto global_col = phi.local_to_global({r, 0}, {s, t})[0];
                    col_buffer(target_rank)[global_col] = mat.at({r, q});
                }
            }
        }
        world.sync();

        // (11)
        for (auto i = 0; i < phi.local_size({s, t})[0]; ++i) {
            for (auto j = 0; j < phi.local_size({s, t})[1]; ++j) {
                auto [a, b] = phi.local_to_global({i, j}, {s, t});
                mat.at({i, j}) -= row_buffer[a] * col_buffer[b];
            }
        }
    }
}

template <typename T>
void spy(matrix<T>& mat) {
    auto& world = mat.world();
    auto& phi = mat.partitioning();
    auto [s, t] = bulk::util::unflatten<2>(phi.grid(), world.rank());

    auto xs =
        bulk::coarray<T>(world, world.rank() == 0 ? mat.cols() * mat.rows() : 0);

    auto x = phi.local_size(world.rank())[0];
    auto y = phi.local_size(world.rank())[1];

    for (int i = 0; i < x; ++i) {
        for (int j = 0; j < y; ++j) {
            xs(0)[bulk::util::flatten<2>(
                phi.global_size(), phi.local_to_global({i, j}, {s, t}))] =
                mat.at({i, j});
        }
    }

    world.sync();

    if (world.rank() == 0) {
        for (int i = 0; i < mat.rows(); ++i) {
            std::cout << "[ " << std::fixed << std::setprecision(2);
            for (int j = 0; j < mat.cols(); ++j) {
                std::cout
                    << xs[bulk::util::flatten<2>(phi.global_size(), {i, j})]
                    << " ";
            }
            std::cout << "]\n";
        }
    }
}

} // namespace psc
