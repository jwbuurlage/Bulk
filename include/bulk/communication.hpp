#pragma once

#include <bulk/variable.hpp>
#include <bulk/array.hpp>
#include <bulk/future.hpp>


namespace bulk {

/// /brief Put a value into a variable held by a (remote) processor
/// /param processor the id of a remote processor holding the variable
/// /param value the new value of the variable
template <typename T, typename Hub>
void put(int processor, T value, var<T, Hub>& the_variable) {
    the_variable.hub().provider().internal_put_(processor, &value, &the_variable.value(),
                                          sizeof(T), 0, 1);
}

template <typename T, typename Hub>
void put(int processor, T value, array<T, Hub>& the_array, int offset = 0,
         int count = 1) {

    the_array.hub().provider().internal_put_(processor, &value, the_array.data(), sizeof(T),
                           offset, count);
}

/// /brief Get a value from a variable held by a (remote) processor
/// /param processor the id of a remote processor holding the variable
/// /param the_variable the variable to obtain the value from
template <typename T, typename Hub>
future<T, Hub> get(int processor, var<T, Hub>& the_variable) {
    future<T, Hub> result(the_variable.hub());
    the_variable.hub().provider().internal_get_(
        processor, &the_variable.value(), result.buffer_, sizeof(T), 0, 1);
    return result;
}

} // namespace bulk
