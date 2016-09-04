#pragma once
#include "coarray.hpp"
#include "messages.hpp"
#include "variable.hpp"
#include "world_provider.hpp"
#include <bulk/world.hpp>
#include <bulk/array.hpp>
#include <bulk/future.hpp>

namespace bulk {
namespace cpp {

class backend {
  public:
    using implementation = world_provider;

    template <typename Tag, typename Content>
    using queue_type = queue<Tag, Content, bulk::world<backend>>;

    template <typename T>
    using var_type = var<T, bulk::world<backend>>;

    template <typename T>
    using future_type = bulk::future<T, bulk::world<backend>>;

    template <typename T>
    using coarray_type = coarray<T, bulk::world<backend>>;

    template <typename T>
    using array_type = bulk::array<T, bulk::world<backend>>;
};

} // namespace mpi
} // namespace bulk
