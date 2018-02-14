#pragma once

// Function written in assembly
// Returns the number of clockcycles since the previous call
extern "C" unsigned int get_raw_time();

namespace bulk {
namespace epiphany {

void memcpy(void* dest, const void* source, unsigned int nbytes);

void print(const char* format, ...)
    __attribute__((__format__(__printf__, 1, 2)));

void* ext_malloc(unsigned int nbytes);

void* malloc(unsigned int nbytes);

void free(void* ptr);

float host_time();

// For debugging purposes, prints memory allocation status
void print_malloc_info_();
}
}
