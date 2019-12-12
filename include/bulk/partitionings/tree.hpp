#include "partitioning.hpp"

namespace bulk {

/**
 * A binary-space partitioning (aptly abbreviated BSP, of course because of its
 * use for parallel computations).
 *
 *              (a_0, d_0)
 *              /        \
 *       (a_10, d_10) (a_11, d_11)
 *       /      \       /     \
 *      ...     ...    ...    ...
 *       |       |      |      |
 * (a_n0, d_n0) ...    ...  (a_n(p/2), d_n(p/2))
 *
 * represented as binary tree of splits.
 */
template <int D>
class tree_partitioning : public rectangular_partitioning<D, 1> {
  public:
    using rectangular_partitioning<D, 1>::local_size;
    using rectangular_partitioning<D, 1>::origin;
    using rectangular_partitioning<D, 1>::local;

    /**
     * Constructs a cyclic partitioning in nD.
     *
     * `grid`: the number of processors in each dimension
     * `data_size`: the global number of processors along each axis
     */
    tree_partitioning(index_type<D> data_size, int procs, util::binary_tree<util::split>&& splits)
    : rectangular_partitioning<D, 1>(data_size, {procs}), splits_(std::move(splits)) {
        // resize extents and origins
        origins_.resize(procs);
        extents_.resize(procs);

        // compute origins and extents here
        for (int proc = 0; proc < procs; ++proc) {
            index_type<D> left_bound{};
            index_type<D> right_bound = this->global_size_;

            auto node = splits_.root.get();
            int depth = 0;
            while (node) {
                if (proc & (1 << depth)) {
                    left_bound[node->value.d] = node->value.a + 1;
                    node = node->right.get();
                } else {
                    right_bound[node->value.d] = node->value.a + 1;
                    node = node->left.get();
                }
                ++depth;
            }
            origins_[proc] = left_bound;
            for (int d = 0; d < D; ++d) {
                extents_[proc][d] = right_bound[d] - left_bound[d];
            }
        }
    }

    /** Compute the local indices of a element using its global indices */
    index_type<D> local(index_type<D> index) override final {
        auto t = this->owner(index);
        for (int d = 0; d < D; ++d) {
            index[d] -= origins_[t][d];
        }
        return index;
    }

    /** The total number of elements along each axis on the processor index with
     * `idxs...` */
    index_type<D> local_size(index_type<1> idxs) override final {
        return extents_[idxs.get()];
    }

    index_type<1> multi_owner(index_type<D> xs) override final {
        // we encode the path to the final volume as a bit pattern, that will be
        // the processor id
        auto node = splits_.root.get();
        int proc = 0;
        int depth = 0;
        while (node) {
            if (xs[node->value.d] <= node->value.a) {
                node = node->left.get();
            } else {
                proc += 1 << depth;
                node = node->right.get();
            }
            ++depth;
        }
        return {proc};
    }

    index_type<D> origin(int t) const override { return origins_[t]; }

    const auto& splits() const { return splits_; }

  private:
    util::binary_tree<util::split> splits_;
    std::vector<index_type<D>> origins_;
    std::vector<index_type<D>> extents_;
};

} // namespace bulk
