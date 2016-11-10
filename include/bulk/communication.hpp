#pragma once

/**
 * \file communication.hpp
 *
 * This header provides the main functions that are used to communicate using
 * distributed variables.
 */

#include "future.hpp"
#include "world.hpp"

namespace bulk {

/**
 * Put a value into a variable held by a (remote) processor
 *
 * \param processor the id of a remote processor holding the variable
 * \param value the new value of the variable
 */
template <typename T, typename var_type>
void put(int processor, T value, var_type& the_variable) {
    the_variable.world().implementation().internal_put_(processor, value,
                                                        the_variable);
}

template <typename T, typename array_type>
void put(int processor, T value, array_type& the_array, int offset,
         int count = 1) {
    the_array.world().implementation().internal_put_(
        processor, &value, the_array.data(), sizeof(T), offset, count);
}

// FIXME: bulk::get should be specialized for different backends.
// It is however not allowed to do partial template specialization
// for functions. Instead we could try something like the answer in
// http://stackoverflow.com/questions/5101516/why-function-template-cannot-be-partially-specialized

/**
 * Get a value from a variable held by a (remote) processor
 *
 * \param processor the id of a remote processor holding the variable
 * \param the_variable the variable to obtain the value from
 *
 * \returns a `future` object that will contain the current remote value,
 * starting from the next superstep.
 */
template <typename T, class World, template <typename, class> class var_type>
future<T, World> get(int processor, var_type<T, World>& the_variable) {
    // FIXME: can we rely on RVO?
    future<T, World> result(the_variable.world());
    the_variable.world().implementation().internal_get_(processor, the_variable,
                                                        result.value());
    return result;
}

template <typename T, class World, template <typename, class> class array_type>
future<T, World> get(int processor, array_type<T, World>& the_array,
                     int offset, int count = 1) {
    // FIXME: can we rely on RVO?
    future<T, World> result(the_array.world());
    the_array.world().implementation().internal_get_(
        processor, the_array.data(), &result.value(), sizeof(T), offset, count);
    return result;
}

}  // namespace bulk
