#include "common.hpp"
#include <bulk/backends/epiphany/host.hpp>
#include <bulk/environment.hpp>
#include <cstdio>
#include <iostream>
#include <vector>

using std::vector;

void print_matrix(float* matrix, int size) {
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j)
            printf("%5.1f ", matrix[i * size + j]);
        printf("\n");
    }
}

// To check the result C
// Assumes C is set to zero
void matrix_multiply_add(float* A, float* B, float* C, int size) {
    for (int i = 0; i < size; i++)
        for (int j = 0; j < size; j++)
            for (int k = 0; k < size; k++)
                C[i * size + j] += A[i * size + k] * B[k * size + j];
}

int main(int argc, char** argv) {
    bulk::environment<bulk::epiphany::provider> env;
    if (!env.provider().is_valid())
        return 1;
    if (env.available_processors() != N * N) {
        std::cerr << "Error, not exactly " << N * N
                  << " processors available.\n";
    }

    int matrix_size = 0;
    int matrix_bytes = 0;
    int block_count = 0;

    //matrix_size = BLOCK_SIZE * 8;
    matrix_size = SCRIPT_MATRIX_SIZE;
    int M = matrix_size / BLOCK_SIZE;
    matrix_bytes = matrix_size * matrix_size * sizeof(float);
    block_count = matrix_size / BLOCK_SIZE;

    printf("Multiplying two %i x %i matrices\n", matrix_size, matrix_size);
    printf("Full matrix consists of %dx%d = %d superblocks of size %dx%d\n",
            M, M, M * M, BLOCK_SIZE, BLOCK_SIZE);
    printf("One superblock contains %d core-blocks of size %dx%d\n",
            N * N, CORE_BLOCK_SIZE, CORE_BLOCK_SIZE);

    // Allocate matrices
    vector<float> A(matrix_size * matrix_size);
    vector<float> B(matrix_size * matrix_size);
    vector<float> C(matrix_size * matrix_size);

    // Fill matrices
    for (int i = 0; i < matrix_size; i++) {
        for (int j = 0; j < matrix_size; j++) {
            A[i * matrix_size + j] = (float)(i + j);
            B[i * matrix_size + j] = (float)j;
            C[i * matrix_size + j] = 0.0f;
        }
    }

    // Save matrix into different arrays: the streams
    vector<vector<float>> stream_A(N * N);
    vector<vector<float>> stream_B(N * N);
    vector<vector<float>> stream_C(N * N);
    int cur_index_A[N * N];
    int cur_index_B[N * N];

    for (int i = 0; i < N * N; i++) {
        cur_index_A[i] = 0;
        cur_index_B[i] = 0;
        stream_A[i].resize((matrix_size * matrix_size) / (N * N));
        stream_B[i].resize((matrix_size * matrix_size) / (N * N));
        stream_C[i].resize((matrix_size * matrix_size) / (N * N));
    }

    // Loop over blocks
    for (int block_Y = 0; block_Y < block_count; block_Y++) {
        for (int block_X = 0; block_X < block_count; block_X++) {
            // When A is divided in BLOCK_SIZE * BLOCK_SIZE blocks
            // we now want the block at block_Y, block_X
            // So that block's top-left element is
            // (i,j) = (block_Y * BLOCK_SIZE , block_X * BLOCK_SIZE)
            // Now loop i,j from 0 to BLOCK_SIZE and use
            // A[ (block_Y * BLOCK_SIZE + j) * matrix_size + (block_X *
            // BLOCK_SIZE + j) ]
            //
            // Within this block, we want to partition into 16 * 16 smaller
            // blocks, done in the following loop

            for (int i = 0; i < BLOCK_SIZE; i++) {
                for (int j = 0; j < BLOCK_SIZE; j++) {
                    float element_A =
                        A[(block_Y * BLOCK_SIZE + i) * matrix_size +
                          (block_X * BLOCK_SIZE + j)];
                    float element_B =
                        B[(block_X * BLOCK_SIZE + i) * matrix_size +
                          (block_Y * BLOCK_SIZE + j)];
                    // i,j are coordinates within the block
                    int X = i / CORE_BLOCK_SIZE;
                    int Y = j / CORE_BLOCK_SIZE;
                    //
                    // Target processor: P(i,j) gets block_A(i,i+j) and
                    // block_B(i+j,j)
                    //
                    // block_A(X,Y) goes to P(X,Y-X)
                    // block_B(X,Y) goes to P(X-Y,Y)
                    //
                    // P(i,j) has PID 4*i + j
                    int pidA = 4 * X + ((Y - X + N) % N);
                    int pidB = 4 * ((X - Y + N) % N) + Y;

                    stream_A[pidA][cur_index_A[pidA]++] = element_A;
                    stream_B[pidB][cur_index_B[pidB]++] = element_B;
                }
            }
        }
    }

    // Debug
    if (matrix_size <= 32) {
        printf("Matrix A:\n");
        print_matrix(&A[0], matrix_size);
        printf("\n");
        printf("Correct matrix C:\n");
        matrix_multiply_add(&A[0], &B[0], &C[0], matrix_size);
        print_matrix(&C[0], matrix_size);
        // Set to zero again
        for (int i = 0; i < matrix_size * matrix_size; ++i)
            C[i] = 0.0f;
        printf("\n\n");
    }

    for (int i = 0; i < N * N; i++) {
        env.provider().create_stream(&stream_A[i][0], matrix_bytes / (N * N),
                                     matrix_bytes / (N * N));
        env.provider().create_stream(&stream_B[i][0], matrix_bytes / (N * N),
                                     matrix_bytes / (N * N));
        env.provider().create_stream(&stream_C[i][0], matrix_bytes / (N * N),
                                     matrix_bytes / (N * N));
    }

    // Timer
    struct timespec ts_start, ts_end;
    clock_gettime(CLOCK_MONOTONIC, &ts_start);

    env.spawn(N * N, "e_cannon.elf");

    clock_gettime(CLOCK_MONOTONIC, &ts_end);
    float time_elapsed =
        (ts_end.tv_sec - ts_start.tv_sec +
         (ts_end.tv_nsec - ts_start.tv_nsec) * 1.0e-9);

    std::cout << "spmd time in seconds: " << time_elapsed << std::endl;

    // Gather C
    // Loop over blocks
    // everything in row-major order
    int cur_index[N * N] = {0};
    for (int block = 0; block < block_count * block_count; ++block) {
        int blockI = block / block_count;
        int blockJ = block % block_count;
        int baseColumn = blockJ * BLOCK_SIZE;
        int baseRow = blockI * BLOCK_SIZE;
        for (int proc = 0; proc < N * N; ++proc) {
            int s = proc / N;
            int t = proc % N;
            int coreBlockColumn = baseColumn + t * CORE_BLOCK_SIZE;
            int coreBlockRow = baseRow + s * CORE_BLOCK_SIZE;
            for (int i = 0; i < CORE_BLOCK_SIZE; ++i) {
                for (int j = 0; j < CORE_BLOCK_SIZE; ++j) {
                    C[(coreBlockRow + i) * matrix_size + coreBlockColumn + j] =
                        stream_C[proc][cur_index[proc]++];
                }
            }
        }
    }

    if (matrix_size <= 32) {
        printf("Result:\n");
        print_matrix(&C[0], matrix_size);
        printf("\n");
    }

    printf("Result: C[n - 1, n - 1] = %.1f\n", C[matrix_size * matrix_size - 1]);

    float correct = 0.0f;
    for (int i = 0; i < matrix_size; i++)
        correct += A[(matrix_size - 1) * matrix_size + i] *
                   B[i * matrix_size + (matrix_size - 1)];
    printf("Correct last element: %.1f\n", correct);

    return 0;
}
