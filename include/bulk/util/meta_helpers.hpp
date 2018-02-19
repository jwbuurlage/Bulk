#pragma once

#include <type_traits>
#include <utility>
#include <vector>

namespace bulk::meta {

template <typename T>
struct representation {
    static_assert(std::is_trivially_copyable<T>::value,
                  "Only trivially copyable types are supported as `T` for "
                  "distributed variables and queues");
    using type = T;
};

template <typename T>
struct representation<T[]> {
    using type = std::vector<T>;
};

template <>
struct representation<std::string> {
    using type = std::string;
};

// Partial specialization of alias templates is not allowed, so we need this
// indirection
template <typename Enable, typename... Ts>
struct message_t;

template <typename... Ts>
struct message_t<typename std::enable_if_t<(sizeof...(Ts) > 1)>, Ts...> {
    std::tuple<typename representation<Ts>::type...> content;
};

template <typename T>
struct message_t<void, T> {
    typename representation<T>::type content;
};

template <typename... Ts>
struct message : public message_t<void, Ts...> {};

} // namespace bulk::meta
