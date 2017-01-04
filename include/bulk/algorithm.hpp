#pragma once

/**
 * \file algorithm.hpp
 *
 * This header providers various high-level functions to perform common
 * algorithms and  communication patterns found in bulk-synchronous parallel
 * programs.
 */

#include <vector>

#include "world.hpp"
#include "coarray.hpp"
#include "communication.hpp"


namespace bulk {

/**
 * Perform a left-associative fold over a distributed variable.
 *
 * This function applies a function to the images of a variable. This function
 * should be called in the same step on each processor.
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
template <typename T, typename Func>
T foldl(var<T>& x, Func f, T start_value = 0) {
    auto& world = x.world();
    T result = start_value;

    // FIXME: This should be done with puts instead of gets,
    // which are faster on almost every platform

    // allocate space to store each remote value locally
    std::vector<bulk::future<T>> images;
    for (int t = 0; t < world.active_processors(); ++t) {
        // obtain the remote values
        images.push_back(bulk::get<T>(t, x));
    }
    world.sync();
    for (int t = 0; t < world.active_processors(); ++t) {
        // apply f iteratively to the current value, and each remote value
        f(result, images[t].value());
    }
    return result;
}

/**
 * Creates a co-array with images holding the given value on each processor.
 *
 * This function takes an argument, and writes it to the appropriate element on
 * each remote processor. This function should be called in the same step on
 * each processor.
 *
 * \tparam T the type of the value to put in the co-array
 * \tparam World the world in which the communication takes place
 *
 * \param value the value to write to each remote processor
 * \param world the world of the targetted processors
 *
 * \returns a co-array containing on each processor the argument given by each
 * other processor.
 */
template <typename T>
bulk::coarray<T> gather_all(bulk::world& world, T value) {
    bulk::coarray<T> xs(world, world.active_processors());

    for (int t = 0; t < world.active_processors(); ++t) {
        xs(t)[world.processor_id()] = value;
    }

    world.sync();

    return xs;
}

} // namespace bulk
