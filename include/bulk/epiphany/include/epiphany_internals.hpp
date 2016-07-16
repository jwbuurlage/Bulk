#pragma once

// Use this define to place functions or variables in external memory
// TEXT is for functions and normal variables
// RO is for read only globals
#define EXT_MEM_TEXT __attribute__((section("NEW_LIB_RO")))
#define EXT_MEM_RO __attribute__((section("NEW_LIB_WR")))

#include "combuf.hpp"

#include <cstdint>

namespace bulk {
namespace epiphany {

constexpr combuf* combuf_ = (combuf*)E_COMBUF_ADDR;

// Update the type below if this value changes
constexpr int MAX_VARS = 20;

// Update pid type if NPROCS changes
typedef int8_t pid_t;
typedef uint8_t var_id_t;

}
}
