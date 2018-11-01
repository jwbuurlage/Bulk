#pragma once

/**
 * \file algorithm.hpp
 *
 * This header provides various high-level functions to perform common
 * algorithms and communication patterns commonly found in bulk-synchronous
 * parallel programs.
 */

#include <vector>

#include "coarray.hpp"
#include "communication.hpp"
#include "world.hpp"

namespace bulk {

/**
 * Create a coarray with images holding the given value on each processor.
 *
 * This function takes an argument, and writes it to the appropriate element on
 * each remote processor.
 *
 * The cost is `p * g + l`.
 *
 * \tparam T the type of the value to put in the coarray
 *
 * \param value the value to write to each remote processor
 * \param world the world in which the communication takes place
 *
 * \returns a coarray containing on each processor the argument given by each
 * other processor.
 */
template <typename T>
bulk::coarray<T> gather_all(bulk::world& world, T value) {
    bulk::coarray<T> xs(world, world.active_processors());

    for (int t = 0; t < world.active_processors(); ++t) {
        xs(t)[world.rank()] = value;
    }

    world.sync();

    return xs;
}

/**
 * Perform a left-associative fold over a distributed variable.
 *
 * This function applies a function to the images of a variable.
 *
 * The cost is `F * p + p * g + l`, where `F` is the number of
 * flops performed during a single call to `f`.
 *
 * \tparam T the type of the value held by \c x.
 * \tparam Func the binary function to apply to the images of \c x.
 *
 * \param x the variable to fold over
 * \param f a binary function that takes two arguments of type `T`.
 *
 * \returns the result of the expression
 *              \f[ f(f(f(f(x(0), x(1)), x(2)), ...), x(p-1)). \f]
 *          which is computed at each core.
 */
template <typename T, typename Func, typename S = T>
S foldl(var<T>& x, Func f, S start_value = {}) {
    auto& world = x.world();
    auto result = start_value;

    auto images = bulk::gather_all(world, x.value());

    for (int t = 0; t < world.active_processors(); ++t) {
        // apply f iteratively to the current value, and each remote value
        f(result, images[t]);
    }
    return result;
}

} // namespace bulk
