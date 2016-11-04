#pragma once

#include <type_traits>

namespace bulk {

template <bool...>
struct bool_pack {};

template <bool... bs>
using all_true = std::is_same<bool_pack<bs..., true>, bool_pack<true, bs...>>;

template <class R, class... Ts>
using are_all_convertible = all_true<std::is_convertible<Ts, R>::value...>;

template <int count, class R, class... Ts>
using count_of_type =
    all_true<sizeof...(Ts) == count, are_all_convertible<R, Ts...>::value>;

}  // namespace bulk
