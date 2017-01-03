#pragma once

/**
 * \file communication.hpp
 *
 * This header provides the main functions that are used to communicate using
 * distributed variables.
 */

#include "future.hpp"
#include "world.hpp"
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
    v.world().put_(processor, &value, sizeof(T), v.id());
}

template <typename T>
void put(int processor, T* values, array<T>& a, int offset, int count = 1) {
    a.world().put_(processor, values, sizeof(T), a.id(), offset, count);
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
    future<T> result(v.world());
    v.world().get_(processor, v.id(), sizeof(T), result.value());
    return result;
}

template <typename T>
future<T> get(int processor, array<T>& a, int offset, int count = 1) {
    future<T> result(a.world());
    a.world().get_(processor, a.id(), sizeof(T), &result.value(), offset,
                   count);
    return result;
}

}  // namespace bulk
