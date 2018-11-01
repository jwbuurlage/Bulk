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

        auto partitioning = bulk::cyclic_partitioning<1>({size}, {p});

        // vectors
        auto v = psc::vector<int>(world, partitioning);
        auto w = psc::vector<int>(world, partitioning);

        v += w;
        auto u = v + w;
        auto alpha = psc::dot(v, w);
        world.log("%i", alpha);

        // sorting a random vector
        auto block = bulk::block_partitioning<1>({size}, {p});
        auto x = psc::vector<int>(world, partitioning);
        for (auto i = 0; i < (int)x.size(); ++i) {
            x[i] = rand();
        }

        auto sorted_x = psc::sort(x);

        world.log("%i / %i", sorted_x[sorted_x.size() / 2],
                  sorted_x[sorted_x.size() / 2 + 1]);

        auto n = 10;
        auto phi = bulk::cyclic_partitioning<2, 2>({n, n}, {M, N});
        auto A = psc::matrix<float>(world, phi);
        for (int i = 0; i < n; ++i) {
            if (phi.owner({i, i}) == world.rank()) {
                A.at(phi.local({i, i})) = 1;
            }
        }

        psc::spy(A);
        psc::lu(A);
        psc::spy(A);
    });

    return 0;
}
