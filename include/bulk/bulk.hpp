#pragma once

/**
 * \file bulk.hpp
 *
 * This header is added for convenience, it includes all of the headers in this
 * project. To reduce compile times, users may wish to include only specific
 * headers.
 */

#include <bulk/algorithm.hpp>
#include <bulk/array.hpp>
#include <bulk/coarray.hpp>
#include <bulk/communication.hpp>
#include <bulk/environment.hpp>
#include <bulk/future.hpp>
#include <bulk/messages.hpp>
#include <bulk/partitioned_array.hpp>
#include <bulk/world.hpp>

#include <bulk/partitionings/block.hpp>
#include <bulk/partitionings/cyclic.hpp>
#include <bulk/partitionings/partitioning.hpp>
#include <bulk/partitionings/tree.hpp>

#include <bulk/util/binary_tree.hpp>
#include <bulk/util/fit.hpp>
#include <bulk/util/indices.hpp>
#include <bulk/util/report.hpp>
#include <bulk/util/timer.hpp>

namespace experimental {}
