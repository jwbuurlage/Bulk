#pragma once
#include <bulk/world.hpp>
#include <bulk/future.hpp>
#include <bulk/coarray.hpp>
#include <bulk/array.hpp>
#include <bulk/variable_direct.hpp>
#include <bulk/messages.hpp>
#include "messages.hpp"
#include "coarray.hpp"
#include "world_provider.hpp"

#include "epiphany_internals.hpp"

namespace bulk {
namespace epiphany {

class backend {
  public:
    using implementation = bulk::epiphany::world_provider;

    template <typename TTag, typename TContent>
    using message_container_type = message_container<TTag, TContent>;

    template <typename T>
    using var_type = bulk::var_direct<T, bulk::world<backend>>;

    template <typename T>
    using future_type = bulk::future<T, bulk::world<backend>>;

    template <typename T>
    using coarray_type = bulk::epiphany::coarray<T, bulk::world<backend>>;

    template <typename T>
    using array_type = bulk::array<T, bulk::world<backend>>;
};

extern bulk::world<bulk::epiphany::backend> world;

} // namespace epiphany
} // namespace bulk
