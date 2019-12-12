/**
 * A partitioned array is a co-array with a special structure. Locally it
 * behaves like a nD array, indexable using `local`. It also knows about the
 * global structure through its associated partitioning.
 * - The data is stored linearly.
 * - A partitioning is defined in a general way, and contains functions for
 * owner and local index lookup, as well as flattening of multi-indices.
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
 * internally we still resort to arrays. This leads to duplicated
 * implementations in the partitioning objects and should be fixed.
 *
 * As it is, this is probably over-engineered and will see major revisions.
 */

#include <array>

#include "coarray.hpp"
#include "future.hpp"
#include "partitionings/partitioning.hpp"

namespace bulk {

/**
 * A partitioned array is a distributed array with an associated partitioning.
 *
 * It is used to retain the notion of a 'global index' without refering
 * specifically to a partitioning.
 *
 * TODO: think about template argument storage class (coarray by default)
 * TODO: this about specializing for 'rectangular partitionings'
 */
template <typename T, int D, int G>
class partitioned_array {
  public:
    /** Construct a partitioned array from a given partitioning. */
    partitioned_array(bulk::world& world, multi_partitioning<D, G>& part)
    : world_(world), partitioning_(part),
      data_(world, partitioning_.local_count(world.rank())) {
        multi_id_ = util::unflatten<D>(partitioning_.grid(), world.rank());
    }

    /** Get an image to a (possibly remote) element using a global index. */
    auto global(index_type<D> index) {
        auto owner = partitioning_.owner(index);
        auto local_idx = partitioning_.local(index);
        return data_(owner)[util::flatten<D>(partitioning_.local_size(owner), local_idx)];
    }

    /** Get an element using its local index. */
    T& local(index_type<D> index) {
        return data_[util::flatten<D>(partitioning_.local_size(multi_id_), index)];
    }

    /// ditto
    const T& local(index_type<D> index) const {
        return data_[util::flatten<D>(partitioning_.local_size(multi_id_), index)];
    }

    /** Get a reference to the world of the array. */
    auto world() const { return world_; }

  private:
    index<D> multi_id_;

    // world in which this array resides
    bulk::world& world_;

    // underlying partitioning
    multi_partitioning<D, G>& partitioning_;

    // linear storage
    bulk::coarray<T> data_;
};

} // namespace bulk
