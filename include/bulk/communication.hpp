#pragma once

#include <bulk/variable.hpp>
#include <bulk/array.hpp>
#include <bulk/future.hpp>


namespace bulk {

/// /brief Put a value into a variable held by a (remote) processor
/// /param processor the id of a remote processor holding the variable
/// /param value the new value of the variable
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

/// /brief Get a value from a variable held by a (remote) processor
/// /param processor the id of a remote processor holding the variable
/// /param the_variable the variable to obtain the value from
template <typename T, typename World>
future<T, World> get(int processor, var<T, World>& the_variable) {
    future<T, World> result(the_variable.world());
    the_variable.world().provider().internal_get_(
        processor, &the_variable.value(), result.buffer_, sizeof(T), 0, 1);
    return result;
}

} // namespace bulk
