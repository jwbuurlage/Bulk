#pragma once

/**
 * \file communication.hpp
 *
 * This header provides the main functions that are used to communicate using
 * distributed variables.
 */

#include "world.hpp"
#include "future.hpp"
#include "variable.hpp"

namespace bulk {

/**
 * Put a value into a variable held by a (remote) processor
 *
 * \param processor the id of a remote processor holding the variable
 * \param value the new value of the variable
 * \param v the variable to put the value into
 */
template <typename T>
void put(int processor, T value, var<T>& v) {
    v(processor) = value;
}

template <typename T>
void put(int processor, T* values, array<T>& a, int offset, int count = 1) {
    a.put(processor, values, offset, count);
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
template <typename T>
future<T> get(int processor, var<T>& v) {
    return v(processor).get();
}

template <typename T>
future<T> get(int processor, array<T>& a, int offset, int count = 1) {
    return a.get(processor, offset, count);
}

}  // namespace bulk
