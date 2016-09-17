#include <bulk/world.hpp>
#include <bulk/backends/epiphany/epiphany.hpp>
#include "common.hpp"

using bulk::epiphany::world;
using bulk::epiphany::print;

int s, p;

// This should be replaced by the assembly version
void matrix_multiply_add(float* A, float* B, float* C) {
    for (int i = 0; i < CORE_BLOCK_SIZE; i++)
        for (int j = 0; j < CORE_BLOCK_SIZE; j++)
            for (int k = 0; k < CORE_BLOCK_SIZE; k++)
                C[i * CORE_BLOCK_SIZE + j] +=
                    A[i * CORE_BLOCK_SIZE + k] * B[k * CORE_BLOCK_SIZE + j];
}

using bulk::epiphany::coarray;

int main() {
    s = world.processor_id();
    p = world.active_processors();

    int si = s / N;
    int sj = s % N;
    int M = SCRIPT_MATRIX_SIZE / BLOCK_SIZE;

    int a_neighbor = si * N + ((sj + 1) % N);
    int b_neighbor = ((si + 1) % N) * N + sj;

    // 5 buffers
    float* a_data[2];
    float* b_data[2];
    float* c_data;

    // neighbor buffer locations
    float* neighbor_a_data[2];
    float* neighbor_b_data[2];

    // Allocate local buffers
    a_data[0] = new float[CORE_BLOCK_SIZE * CORE_BLOCK_SIZE];
    a_data[1] = new float[CORE_BLOCK_SIZE * CORE_BLOCK_SIZE];
    b_data[0] = new float[CORE_BLOCK_SIZE * CORE_BLOCK_SIZE];
    b_data[1] = new float[CORE_BLOCK_SIZE * CORE_BLOCK_SIZE];
    c_data = new float[CORE_BLOCK_SIZE * CORE_BLOCK_SIZE];

    // Set C to zero
    // For safety and easier debugging, set A,B to zero
    for (int i = 0; i < CORE_BLOCK_SIZE * CORE_BLOCK_SIZE; ++i) {
        a_data[0][i] = -1;
        a_data[1][i] = -1;
        b_data[0][i] = -1;
        b_data[1][i] = -1;
        c_data[i] = 0;
    }

    // Register locations of buffers in order to get neighbor locations
    // Alternative is to use coarrays for a_data but current type system
    // does not allow this in an easy way
    int a_id_0 =
        bulk::epiphany::state.register_location_(a_data[0], CORE_BLOCK_BYTES);
    int a_id_1 =
        bulk::epiphany::state.register_location_(a_data[1], CORE_BLOCK_BYTES);
    int b_id_0 =
        bulk::epiphany::state.register_location_(b_data[0], CORE_BLOCK_BYTES);
    int b_id_1 =
        bulk::epiphany::state.register_location_(b_data[1], CORE_BLOCK_BYTES);

    neighbor_a_data[0] =
        (float*)bulk::epiphany::state.get_direct_address_(a_neighbor, a_id_0);
    neighbor_a_data[1] =
        (float*)bulk::epiphany::state.get_direct_address_(a_neighbor, a_id_1);
    neighbor_b_data[0] =
        (float*)bulk::epiphany::state.get_direct_address_(b_neighbor, b_id_0);
    neighbor_b_data[1] =
        (float*)bulk::epiphany::state.get_direct_address_(b_neighbor, b_id_1);

    // Claim the streams
    bulk::epiphany::stream stream_a(3 * s + 0);
    bulk::epiphany::stream stream_b(3 * s + 1);
    bulk::epiphany::stream stream_c(3 * s + 2);

    // Check if the streams are open
    auto so_far_so_good = bulk::create_var<int>(world);
    so_far_so_good = 1;
    world.sync();
    if (!stream_a || !stream_b || !stream_c) {
        bulk::epiphany::print("Unable to open streams.");
        so_far_so_good(0) = 0;
    }
    world.sync();

    if (so_far_so_good(0)) {

        bulk::epiphany::dma_task dma_task_a;
        bulk::epiphany::dma_task dma_task_b;

        float time1 = bulk::epiphany::host_time();

        // Loop over the blocks (chunks)
        // these are the *global blocks*
        for (int cur_block = 0; cur_block <= M * M * M; cur_block++) {

            if (cur_block != 0) {
                if (cur_block % (M * M) == 0) {
                    stream_b.seek_rel(-(M * M) * CORE_BLOCK_BYTES);
                } else if (cur_block % M == 0) {
                    stream_a.seek_rel(-M * CORE_BLOCK_BYTES);
                }

                if (cur_block % M == 0) {
                    // Send result of C upwards.
                    // Synchronous: wait for completion
                    stream_c.write(c_data, CORE_BLOCK_BYTES, true);
                    world.barrier();

                    if (cur_block == M * M * M) {
                        break;
                    }

                    // Set C to zero
                    for (int i = 0; i < CORE_BLOCK_SIZE * CORE_BLOCK_SIZE; ++i)
                        c_data[i] = 0;
                }
            }

            // Obtain A, B
            // Last bool is `wait_for_completion`
            stream_a.read(a_data[0], CORE_BLOCK_BYTES, true);
            stream_b.read(b_data[0], CORE_BLOCK_BYTES, true);

            int cur = 0;        // computation
            int cur_buffer = 1; // data transfer

            // Now apply the normal Cannon algorithm
            // Multiply this block, by looping over the *core blocks*
            for (int i = 0; i < N; i++) {
                if (i != N - 1) {
                    // Transfer blocks
                    dma_task_a.push(neighbor_a_data[cur_buffer], a_data[cur],
                                    CORE_BLOCK_BYTES, 0);
                    dma_task_b.push(neighbor_b_data[cur_buffer], b_data[cur],
                                    CORE_BLOCK_BYTES, 0);
                }

                // Perform C += A * B
                matrix_multiply_add(a_data[cur], b_data[cur], c_data);

                if (i == N - 1)
                    break;

                // Switch buffers
                cur_buffer = 1 - cur_buffer;
                cur = 1 - cur;

                dma_task_a.wait();
                dma_task_b.wait();

                world.barrier();
            }
        }

        float time2 = bulk::epiphany::host_time();

        stream_a.close();
        stream_b.close();
        stream_c.close();

        if (s == 0)
            print("Timings in seconds:");
        world.sync();
        // Trick to print a float using ints
        float time3 = time2 - time1;
        int time4 = (int)time3;
        int time5 = (int)(1000.0f * (time3 - (float)time4));
        print("%d.%03d", time4, time5);
    }

    // Clean up
    delete[] a_data[0];
    delete[] a_data[1];
    delete[] b_data[0];
    delete[] b_data[1];
    delete[] c_data;

    world.sync();
    return 0;
}
