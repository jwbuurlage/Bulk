#pragma once
#include <limits>

// Use this define to place functions or variables in external memory
// TEXT is for functions and normal variables
// RO is for read only globals
#define EXT_MEM_TEXT __attribute__((section("NEW_LIB_RO")))
#define EXT_MEM_RO __attribute__((section("NEW_LIB_WR")))

#include "combuf.hpp"

#include <cstdint>

namespace bulk {
namespace epiphany {

// casts are officially not allowed in constexpr
// gcc accepts it but clang does not
// constexpr combuf* combuf_ = (combuf*)E_COMBUF_ADDR;
// hence the define. We do not want a normal pointer because
// it will cause an extra pointer lookup in EVERY access
#define combuf_  ((combuf*)E_COMBUF_ADDR)

// Update the type below if this value changes

// Update pid type if NPROCS changes
typedef int8_t pid_t;
typedef uint8_t var_id_t;

constexpr var_id_t VAR_INVALID = std::numeric_limits<var_id_t>::max();
constexpr var_id_t MAX_VARS = 20;

}
}
