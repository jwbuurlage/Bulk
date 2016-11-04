/**
 * A partitioned array is a co-array with a special structure. Locally it
* behaves like a nD array, indexable using `local`. It also knows about the
* global structure through its associated partitioning.
* - The data is stored linearly.
* - A partitioning is defined in a general way, and contains functions for owner
* and local index lookup, as well as flattening of multi-indices.
* - convenient indexing features
*
* Examples of partitionings include:
* - Block partitioning
* - Cyclic partitioning
* - A binary tree of splits, giving a binary (space) partitioning.
* - A set of slices ((2,2,1,1), (3,2,1)), see
* http://dask.pydata.org/en/latest/array-creation.html
* - Custom partitionings, for example cartesian distributions using different
* functions for each axis
*
* Some design decisions that have to be made have to do with inheritence and
* overload functions.
*
* Also, we want to support multi-indices, but dont want to resort to `arrays`
* for everything. Using parameter packs we can do this nicely for the user, but
* internally we still resort to arrays. This leads to duplicated implementations
* in the partitioning objects and should be fixed.
*
* As it is, this is probably over-engineered and will see major revisions.
*/

#include <array>

#include "partitioning.hpp"
#include "coarray.hpp"
#include "future.hpp"
#include "util/meta_helpers.hpp"

namespace bulk {

template <int D, typename... Ts>
using check_dim =
    typename std::enable_if<count_of_type<D, int, Ts...>::value>::type;

/**
 * A partitioned array is a distributed array with an associated partitioning.
 *
 * It is used to retain the notion of a 'global index' without refering
 * specifically to a partitioning.
 *
 * TODO: think about template argument storage class (coarray by default)
 * TODO: this about specializing for 'rectangular partitionings'
 */
template <typename T, int D, typename World>
class partitioned_array {
   public:
    /** Construct a partitioned array from a given partitioning. */
    partitioned_array(World& world, partitioning<D, D>& part)
        : world_(world),
          part_(part),
          data_(world, part_.local_element_count(world.processor_id())) {
        multi_id_ = unflatten<D>(part_.grid(), world.processor_id());
    }

    /** Obtain an image to a (possibly remote) element using a global index. */
    template <typename... Ts, typename = check_dim<D, Ts...>>
    auto global(Ts... index) {
        auto owner = part_.owner({index...});
        auto idx = flatten<D>(part_.local_extent(owner),
                              part_.local_index({index...}));
        return data_(flatten<D>(part_.grid(), part_.owner({index...})))[idx];
    }

    /** Obtain an element using its local index. */
    template <typename... Ts, typename = check_dim<D, Ts...>>
    T& local(Ts... index) {
        return data_[flatten<D>(part_.local_extent(multi_id_), {index...})];
    }

   private:
    std::array<int, D> multi_id_;

    // world in which this array resides
    World& world_;

    // underlying partitioning
    partitioning<D, D>& part_;

    // linear storage
    bulk::coarray<T, World> data_;
};

}  // namespace bulk
