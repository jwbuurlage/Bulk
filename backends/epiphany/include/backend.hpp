#pragma once
#include <bulk/world.hpp>
#include <bulk/future.hpp>
#include <bulk/array.hpp>
#include <bulk/messages.hpp>
#include "messages.hpp"
#include "coarray.hpp"
#include "variable.hpp"
#include "world_provider.hpp"

#include "epiphany_internals.hpp"

namespace bulk {
namespace epiphany {

class backend {
  public:
    using implementation = bulk::epiphany::world_provider;

    template <typename Tag, typename Content>
    using queue_type = queue<Tag, Content, bulk::world<backend>>;

    template <typename T>
    using var_type = bulk::epiphany::var<T, bulk::world<backend>>;

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
