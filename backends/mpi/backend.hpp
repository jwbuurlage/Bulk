#pragma once
#include "messages.hpp"
#include "world_provider.hpp"
#include <bulk/array.hpp>
#include <bulk/coarray.hpp>
#include <bulk/future.hpp>
#include <bulk/variable_indirect.hpp>

namespace bulk {
namespace mpi {

class backend {
  public:
    using implementation = bulk::mpi::world_provider;

    template <typename Tag, typename Content>
    using queue_type = queue<Tag, Content, bulk::world<backend>>;

    template <typename T>
    using var_type = bulk::var_indirect<T, bulk::world<backend>>;

    template <typename T>
    using future_type = bulk::future<T, bulk::world<backend>>;

    template <typename T>
    using coarray_type = bulk::coarray<T, bulk::world<backend>>;

    template <typename T>
    using array_type = bulk::array<T, bulk::world<backend>>;
};

} // namespace mpi
} // namespace bulk
