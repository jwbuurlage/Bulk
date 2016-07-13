#pragma once
#include <stdint.h>

namespace bulk {
namespace epiphany {

constexpr int NPROCS = 16;

// Structures that are shared between ARM and epiphany
// need to use the same alignment
// By default, the epiphany compiler will align structs
// to 8-byte and the ARM compiler will align to 4-byte.
// Since all contents of the structs are 4-byte, we can
// safely use 4-byte packing without losing speed
#pragma pack(push, 4)
typedef struct {
    // Epiphany --> ARM communication
    int8_t syncstate[NPROCS];
    int8_t* syncstate_ptr; // Location on epiphany core
    char msgbuf[128];      // shared by all cores (mutexed)

    // ARM --> Epiphany
    float remotetimer;
    int32_t nprocs;
} combuf;
#pragma pack(pop)

// For info on these external memory addresses, see
// https://github.com/buurlage-wits/epiphany-bsp/wiki/Memory-on-the-parallella

// Sizes within external memory
#define EXTMEM_SIZE 0x02000000 // Total size, 32 MB
#define NEWLIB_SIZE 0x01800000
#define COMBUF_SIZE sizeof(combuf)
#define DYNMEM_SIZE (EXTMEM_SIZE - COMBUF_SIZE - NEWLIB_SIZE)

// Epiphany addresses
#define E_EXTMEM_ADDR 0x8e000000
#define E_COMBUF_ADDR (E_EXTMEM_ADDR + NEWLIB_SIZE)
#define E_DYNMEM_ADDR (E_EXTMEM_ADDR + NEWLIB_SIZE + COMBUF_SIZE)

// Possible values for syncstate
// They start at 1 so that 0 means that the variable was not initialized
enum SYNCSTATE : int8_t {
    UNDEFINED = 0,
    RUN = 1,
    SYNC = 2,
    CONTINUE = 3,
    FINISH = 4,
    INIT = 5,
    EREADY = 6,
    ABORT = 7,
    MESSAGE = 8,
    COUNT
};

} // namespace epiphany
} // namespace bulk
