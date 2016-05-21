#pragma once

/**
 * \file communication.hpp
 *
 * This header provides the main functions that are used to communicate using
 * distributed variables.
 */

#include "variable.hpp"
#include "array.hpp"
#include "future.hpp"


namespace bulk {

/**
 * Put a value into a variable held by a (remote) processor
 *
 * \param processor the id of a remote processor holding the variable
 * \param value the new value of the variable
 */
template <typename T, typename World>
void put(int processor, T value, var<T, World>& the_variable) {
    the_variable.world().provider().internal_put_(processor, &value, &the_variable.value(),
                                          sizeof(T), 0, 1);
}

template <typename T, typename World>
void put(int processor, T value, array<T, World>& the_array, int offset = 0,
         int count = 1) {

    the_array.world().provider().internal_put_(processor, &value, the_array.data(), sizeof(T),
                           offset, count);
}

/**
 * Get a value from a variable held by a (remote) processor
 *
 * \param processor the id of a remote processor holding the variable
 * \param the_variable the variable to obtain the value from
 *
 * \returns a `future` object that will contain the current remote value,
 * starting from the next superstep.
 */
template <typename T, typename World>
future<T, World> get(int processor, var<T, World>& the_variable) {
    future<T, World> result(the_variable.world());
    the_variable.world().provider().internal_get_(
        processor, &the_variable.value(), result.buffer_.get(), sizeof(T), 0, 1);
    return result;
}

} // namespace bulk
