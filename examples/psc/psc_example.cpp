#include <random>

#include <string>

#include <bulk/bulk.hpp>
#include <psc/psc.hpp>

#ifdef BACKEND_MPI
#include <bulk/backends/mpi/mpi.hpp>
using environment = bulk::mpi::environment;
#else
#include <bulk/backends/thread/thread.hpp>
using environment = bulk::thread::environment;
#endif

int main(int argc, char** argv) {
    assert(argc > 2);
    int M = std::atoi(argv[1]);
    int N = std::atoi(argv[2]);
    int p = M * N;

    environment env;

    env.spawn(p, [&](bulk::world& world) {
        auto p = world.active_processors();
        auto size = 1'000'000;

        world.log_once("> Example 1: Basic vector operations");
        auto partitioning = bulk::cyclic_partitioning<1>({size}, {p});

        auto v = psc::vector<int>(world, partitioning, 1);
        auto w = psc::vector<int>(world, partitioning, 1);

        v += w;
        auto u = v + w;
        auto alpha = psc::dot(v, w);
        world.log_once("Inner product: %i", alpha);

        world.log_once("> Example 2: Sorting a random vector");
        auto block = bulk::block_partitioning<1>({size}, {p});
        auto x = psc::vector<int>(world, partitioning);
        for (auto i = 0; i < (int)x.size(); ++i) {
            x[i] = rand();
        }

        auto sorted_x = psc::sort(x);

        world.log("Middle elements of %i: [..., %i, %i, ...]", world.rank(),
                  sorted_x[sorted_x.size() / 2],
                  sorted_x[sorted_x.size() / 2 + 1]);
        world.sync();

        world.log_once(
            "> Example 3: Computing the LU decomposition of a matrix");
        auto n = 4;
        auto phi = bulk::cyclic_partitioning<2, 2>({n, n}, {M, N});
        auto A = psc::matrix<float>(world, phi, 1.0f);
        for (int i = 0; i < n; ++i) {
            if (phi.owner({i, i}) == world.rank()) {
                A.at(phi.local({i, i})) = 2.0f;
            }
        }

        world.log_once("Matrix before LU:");
        psc::spy(A);

        auto pi[[maybe_unused]] = psc::lu(A);

        world.log_once("Matrix after LU:");
        psc::spy(A);
    });

    return 0;
}
