#include <bulk/backends/thread/thread.hpp>
#include <bulk/bulk.hpp>
#include <chrono>
#include <cmath>

using namespace std::chrono;

// Compute C = A * B

using Num = float;
constexpr int matrix_size = 1024;

Num A[matrix_size][matrix_size];
Num B[matrix_size][matrix_size]; // This is tranposed
Num C[matrix_size][matrix_size];

int block_size;

// C(I,J) += A(I,.) * B(.,J)
void multiply_add(int I, int J) {
    for (int i = 0; i < block_size; ++i) {
        for (int j = 0; j < block_size; ++j) {
            // Compute C(I,J)[i][j]
            Num& c = C[I * block_size + i][J * block_size + j];
            for (int k = 0; k < matrix_size; ++k) {
                c += A[I * block_size + i][k] * B[J * block_size + j][k];
            }
        }
    }
}

int main() {
    bulk::thread::environment env;

    for (auto i = 0u; i < matrix_size; ++i) {
        for (auto j = 0u; j < matrix_size; ++j) {
            A[i][j] = i;
            B[i][j] = j;
            C[i][j] = 0;
        }
    }

    std::vector<double> times_ms(env.available_processors());

    using clock = high_resolution_clock;
    auto spawn_begin = clock::now();
    env.spawn(env.available_processors(), [&times_ms](bulk::world& world) {
        int s = world.rank();
        int p = world.active_processors();

        int N = (int)std::sqrt(p);
        if (N * N != p) {
            world.log("Invalid number of processors (not a square).");
            return;
        }
        if (matrix_size % N != 0) {
            world.log("Matrix size is not a multiple of sqrt(processors).");
            return;
        }
        block_size = matrix_size / N;

        int I = s / N;
        int J = s % N;

        auto begin_time = clock::now();

        // Matrices are N*N blocks of size block_size*block_size
        // This thread computes block (I,J) of C
        multiply_add(I, J);

        auto end_time = clock::now();

        times_ms[s] =
            duration<double, std::milli>(end_time - begin_time).count();
    });
    auto spawn_end = clock::now();
    auto spawn_ms =
        duration<double, std::milli>(spawn_end - spawn_begin).count();
    std::cout << "Entire SPMD took " << spawn_ms << " ms.\n";

    std::cout << "Computation was "
              << 2.0f * float(matrix_size * matrix_size * matrix_size) /
                     1000000000.0f
              << " GFlops in total --> "
              << 2.0f * float(matrix_size * matrix_size * matrix_size) /
                     (1000000.0f * spawn_ms)
              << " GFlops/s.\n";

    for (auto i = 0u; i < times_ms.size(); ++i) {
        std::cout << "Computation took " << times_ms[i] << " ms on processor "
                  << i << " --> "
                  << 2.0f * float(block_size * block_size * matrix_size) /
                         (1000.0f * times_ms[i])
                  << " MFlops/s.\n";
    }

    return 0;
}
