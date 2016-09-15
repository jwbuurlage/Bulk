#define N 4

#define SCRIPT_MATRIX_SIZE 480

#define CORE_BLOCK_SIZE 24 // max 32
#define CORE_BLOCK_BYTES (CORE_BLOCK_SIZE * CORE_BLOCK_SIZE * sizeof(float))

#define BLOCK_SIZE (N * CORE_BLOCK_SIZE)
#define BLOCK_BYTES (BLOCK_SIZE * BLOCK_SIZE * sizeof(float))
