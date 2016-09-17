#define N 4

// The matrix size has to be a multiple of BLOCK_SIZE
#define SCRIPT_MATRIX_SIZE 400

// Every core must fit 8 times a buffer of size CORE_BLOCK_BYTES
// A size of 25 means 25*25*4*8 = 20.000 Bytes
// so this might barely fit
#define CORE_BLOCK_SIZE 20 // max 25
#define CORE_BLOCK_BYTES (CORE_BLOCK_SIZE * CORE_BLOCK_SIZE * sizeof(float))

#define BLOCK_SIZE (N * CORE_BLOCK_SIZE)
#define BLOCK_BYTES (BLOCK_SIZE * BLOCK_SIZE * sizeof(float))
