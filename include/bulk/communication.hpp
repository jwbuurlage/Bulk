#pragma once

/**
 * \file communication.hpp
 *
 * This header provides the main functions that are used to communicate using
 * distributed variables.
 */

#include "future.hpp"
#include "variable.hpp"
#include "world.hpp"

namespace bulk {

/**
 * Put a value into a variable held by a (remote) processor
 *
 * The cost is `g`.
 *
 * \param processor the id of a remote processor holding the variable
 * \param value the new value of the variable
 * \param x the variable to put the value into
 */
template <typename T>
void put(int processor, const T& value, var<T>& x) {
    x(processor) = value;
}

/**
 * Get a future to a remote image.
 *
 * The cost is `g`
 *
 * \param processor the id of the remote processor
 * \param x the variable whose image to obtain
 *
 * \returns a `future` to the image.
 */
template <typename T>
future<T> get(int processor, var<T>& x) {
    return x(processor).get();
}

} // namespace bulk
