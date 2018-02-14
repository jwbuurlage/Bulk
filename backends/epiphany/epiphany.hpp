#pragma once

// Temporary fix to compile lambda kernels that use certain functions
#include <cmath>

#include <bulk/algorithm.hpp>
#include <bulk/array.hpp>
#include <bulk/coarray.hpp>
#include <bulk/communication.hpp>
#include <bulk/future.hpp>
#include <bulk/messages.hpp>
#include <bulk/partitioned_array.hpp>
#include <bulk/variable.hpp>
#include <bulk/world.hpp>

#include <bulk/partitionings/partitioning.hpp>
#include <bulk/partitionings/block.hpp>
#include <bulk/partitionings/cyclic.hpp>
#include <bulk/partitionings/tree.hpp>

namespace bulk {
using namespace experimental;
}

//#include <bulk/util/binary_tree.hpp>
//#include <bulk/util/report.hpp>
//#include <bulk/util/timer.hpp>
//#include <bulk/util/indices.hpp>
//#include <bulk/util/fit.hpp>

// Epiphany-specific
#include "include/utility.hpp"
#include "include/stream.hpp"
#include "include/world.hpp"
#include "include/epiphany_internals.hpp"


namespace bulk {
namespace epiphany {

// The global instance of world
extern bulk::epiphany::world world;

} // namespace epiphany
} // namespace bulk

