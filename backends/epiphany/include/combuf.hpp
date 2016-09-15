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

// Streams, created by the host
// Currently, there is always only a contiguous part
// of the stream in external memory
typedef struct {
    // Allocated buffer
    void* buffer;      // points to external memory, epiphany address space
    uint32_t capacity; // amount of allocated memory
    // Data currently in the buffer
    // These two values are ONLY written to by the host
    int32_t offset;      // offset from the start of the stream
    int32_t filled_size; // amount of data currently in the buffer (<= capacity)
    // Processor currently owning the stream or -1 if none
    int32_t pid;
} stream_descriptor;

typedef struct {
    // Epiphany --> ARM communication
    int8_t syncstate[NPROCS];
    int8_t* syncstate_ptr; // Location on epiphany core
    char msgbuf[128];      // shared by all cores (mutexed)

    // ARM --> Epiphany
    float remotetimer;
    int32_t nprocs;
    int32_t nstreams;
    stream_descriptor* streams;
} combuf;
#pragma pack(pop)

// For info on these external memory addresses, see
// https://github.com/buurlage-wits/epiphany-bsp/wiki/Memory-on-the-parallella

// Sizes within external memory
// Combuf is rounded up to multiple of 8
#define EXTMEM_SIZE 0x02000000 // Total size, 32 MB
#define NEWLIB_SIZE 0x01800000
#define COMBUF_SIZE (((sizeof(bulk::epiphany::combuf) + 7) / 8) * 8)
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
    STREAMREQ = 9,
    STREAMWRITE = 10,
    COUNT
};

} // namespace epiphany
} // namespace bulk
