#pragma once

#include <cmath>
#include <iomanip>
#include <numeric>

#include "bulk/bulk.hpp"
#include "vector.hpp"

namespace psc {

template <typename T>
class matrix {
 public:
  matrix(bulk::world& world, bulk::cartesian_partitioning<2, 2>& partitioning,
         T value = 0)
      : world_(world),
        partitioning_(partitioning),
        data_(world_, partitioning.local_count(world_.rank()), value) {}

  T& at(bulk::index_type<2> index) {
    return data_[bulk::util::flatten<2>(
        partitioning_.local_size(partitioning_.multi_rank(world_.rank())),
        index)];
  }

  auto& partitioning() { return partitioning_; };
  auto& world() { return world_; };

  auto rows() { return partitioning_.global_size()[0]; }
  auto cols() { return partitioning_.global_size()[1]; }

 private:
  bulk::world& world_;
  bulk::cartesian_partitioning<2, 2>& partitioning_;
  bulk::coarray<T> data_;
};

template <typename T>
bulk::coarray<size_t> lu(matrix<T>& mat) {
  auto& world = mat.world();
  auto& psi = mat.partitioning();
  auto n = mat.rows();
  auto [s, t] = psi.multi_rank(world.rank()).get();

  auto pivot = bulk::var<T>(world, (T)-1);
  auto row_k = bulk::coarray<T>(world, psi.local_size(1, t));
  auto col_k = bulk::coarray<T>(world, psi.local_size(0, s));

  auto row_swap_k = bulk::coarray<T>(world, psi.local_size(1, t));
  auto row_swap_r = bulk::coarray<T>(world, psi.local_size(1, t));

  auto pi = bulk::coarray<size_t>(world, psi.local_size(0, s));
  // partitioned according to psi_0
  for (auto i = 0ul; i < psi.local_size(0, s); ++i) {
    pi[i] = psi.global(0, s, i);
  }

  // identify the first {l/q} such that global({l/q}) > k
  auto ls = std::vector<size_t>(n);
  auto qs = std::vector<size_t>(n);
  auto ridx = 0ul;
  auto cidx = 0ul;
  for (auto k = 0ul; k < n; ++k) {
    while (psi.global(0, s, ridx) <= k) {
      ridx++;
    }
    while (psi.global(1, t, cidx) <= k) {
      cidx++;
    }
    ls[k] = ridx;
    qs[k] = cidx;
  }

  auto pivot_rows = bulk::coarray<size_t>(world, psi.grid()[0]);
  auto pivot_values = bulk::coarray<T>(world, psi.grid()[0]);
  auto r = bulk::var<size_t>(world);

  for (auto k = 0ul; k < n; ++k) {
    if (static_cast<size_t>(psi.owner(1, k)) == t) {
      // (0) Find maximum local element in column k
      auto r_s = -1;
      auto local_max = std::numeric_limits<T>::min();
      for (auto j = 0ul; j < psi.local_size(0, s); ++j) {
        if (std::abs(mat.at({j, psi.local(1, k)})) > local_max) {
          local_max = std::abs(mat.at({j, psi.local(1, k)}));
          r_s = psi.global(0, s, j);
        }
      }

      // (1) Communicate maximum element with local processor column
      for (auto u = 0ul; u < psi.grid()[0]; ++u) {
        pivot_rows(psi.rank({u, t}))[s] = r_s;
        pivot_values(psi.rank({u, t}))[s] = local_max;
      }

      world.sync();  // (a)

      // (2) Find global maximum
      auto best_index = std::distance(
          pivot_values.begin(),
          std::max_element(pivot_values.begin(), pivot_values.end()));

      // (3) Communicate global maximum with local processor row
      for (auto v = 0ul; v < psi.grid()[1]; ++v) {
        r(psi.rank({s, v})) = pivot_rows[best_index];
      }
    } else {
      world.sync();  // (a)
    }
    world.sync();

    // (4) Communicate permutation update
    if (static_cast<size_t>(psi.owner(0, k)) == s) {
      pi(psi.rank({static_cast<size_t>(psi.owner(0, r)), t}))[psi.local(0, r)] =
          pi[psi.local(0, k)];
    }
    if (static_cast<size_t>(psi.owner(0, r)) == s) {
      pi(psi.rank({static_cast<size_t>(psi.owner(0, k)), t}))[psi.local(0, k)] =
          pi[psi.local(0, r)];
    }
    world.sync();

    // (5) Update local permutation vector
    // ... not necessary

    // (6) Communicate row k and r
    if (static_cast<size_t>(psi.owner(0, k)) == s) {
      for (auto j = 0ul; j < psi.local_size(1, t); ++j) {
        row_swap_k(psi.rank({static_cast<size_t>(psi.owner(0, r)), t}))[j] =
            mat.at({psi.local(0, k), j});
      }
    }
    if (static_cast<size_t>(psi.owner(0, r)) == s) {
      for (auto j = 0ul; j < psi.local_size(1, t); ++j) {
        row_swap_r(psi.rank({static_cast<size_t>(psi.owner(0, k)), t}))[j] =
            mat.at({psi.local(0, r), j});
      }
    }

    world.sync();

    // (7) Locally swap row k and r
    if (static_cast<size_t>(psi.owner(0, k)) == s) {
      for (auto j = 0ul; j < psi.local_size(1, t); ++j) {
        mat.at({psi.local(0, k), j}) = row_swap_r[j];
      }
    }
    if (static_cast<size_t>(psi.owner(0, r)) == s) {
      for (auto j = 0ul; j < psi.local_size(1, t); ++j) {
        mat.at({psi.local(0, r), j}) = row_swap_k[j];
      }
    }
    // (8) Communicate pivot
    if (psi.owner({k, k}) == world.rank()) {
      for (auto u = 0ul; u < psi.grid()[0]; ++u) {
        pivot(psi.rank({u, t})) = mat.at(psi.local({k, k}));
      }
    }
    world.sync();

    // (9) Scale column
    if (static_cast<size_t>(psi.owner(1, k)) == t) {
      // local row index, local col size, loop
      for (auto i = ls[k]; i < psi.local_size(0, s); ++i) {
        mat.at({i, psi.local(1, k)}) /= pivot;
      }
    }

    // (10) Horizontal, communicate column k
    if (static_cast<size_t>(psi.owner(1, k)) == t) {
      for (auto v = 0ul; v < psi.grid()[1]; ++v) {
        auto target_rank = psi.rank({s, v});
        for (auto i = 0ul; i < psi.local_size(0, s); ++i) {
          col_k(target_rank)[i] = mat.at({i, psi.local(1, k)});
        }
      }
    }
    world.sync();

    // (10b) Vertical, communicate row k
    if (static_cast<size_t>(psi.owner(0, k)) == s) {
      for (auto u = 0ul; u < psi.grid()[0]; ++u) {
        auto target_rank = psi.rank({u, t});
        for (auto j = 0ul; j < psi.local_size(1, t); ++j) {
          row_k(target_rank)[j] = mat.at({psi.local(0, k), j});
        }
      }
    }
    world.sync();

    // (11) Update bottom right block
    for (auto i = ls[k]; i < psi.local_size(0, s); ++i) {
      for (auto j = qs[k]; j < psi.local_size(1, t); ++j) {
        mat.at({i, j}) -= col_k[i] * row_k[j];
      }
    }

    world.sync();
  }

  return pi;
}

template <typename T>
void spy(matrix<T>& mat) {
  auto& world = mat.world();
  auto& phi = mat.partitioning();
  auto [s, t] = phi.multi_rank(world.rank()).get();

  auto xs =
      bulk::coarray<T>(world, world.rank() == 0 ? mat.cols() * mat.rows() : 0);

  auto x = phi.local_size(world.rank())[0];
  auto y = phi.local_size(world.rank())[1];

  for (auto i = 0ul; i < x; ++i) {
    for (auto j = 0ul; j < y; ++j) {
      xs(0)[bulk::util::flatten<2>(
          phi.global_size(), phi.global({i, j}, {s, t}))] = mat.at({i, j});
    }
  }

  world.sync();

  if (world.rank() == 0) {
    for (auto i = 0ul; i < mat.rows(); ++i) {
      std::cout << "[ " << std::fixed << std::setprecision(2);
      for (auto j = 0ul; j < mat.cols(); ++j) {
        std::cout << xs[bulk::util::flatten<2>(phi.global_size(), {i, j})]
                  << " ";
      }
      std::cout << "]\n";
    }
    std::cout << "\n";
  }
}

}  // namespace psc
