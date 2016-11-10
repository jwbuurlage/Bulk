#pragma once

#include <array>
#include <iostream>
#include <vector>

#include "util/binary_tree.hpp"

namespace bulk {

/** Free functions for flattening multi-indices in volumes. */
template <int D>
int flatten(std::array<int, D> volume, std::array<int, D> idxs) {
    int flattened = 0;
    int offset = 1;
    for (int d = 0; d < D; ++d) {
        flattened += idxs[d] * offset;
        offset *= volume[d];
    }
    return flattened;
}

template <int D>
std::array<int, D> unflatten(std::array<int, D> volume, int flattened) {
    std::array<int, D> unflattened;
    for (int d = 0; d < D; ++d) {
        unflattened[d] = flattened % volume[d];
        flattened /= volume[d];
    }
    return unflattened;
}

/**
 * Base class for partitionings, defining some common methods.
 *
 * There are three important concepts for partitionings:
 * - the owner of an element
 * - the local index of an element (possibly at a remote processor)
 * - a *flattened* local index*
 *
 * TODO: specialize this for 1D to avoid the arrays
 */
template <int DataDim, int GridDim = DataDim>
class partitioning {
   public:
    partitioning(std::array<int, GridDim> grid,
                 std::array<int, DataDim> data_size)
        : grid_{grid}, size_(data_size) {}

    /** The processor grid for the partitioning. */
    const std::array<int, GridDim>& grid() const { return grid_; };

    /** The global size of the partitioning. */
    const std::array<int, DataDim>& global_size() const { return size_; };

    /** Compute the owner of a element using its global indices */
    virtual std::array<int, GridDim> owner(std::array<int, DataDim> xs) = 0;

    /** Compute the owner of a element using its global indices */
    virtual std::array<int, DataDim> local_index(
        std::array<int, DataDim> xs) = 0;

    /** The total number of elements along each axis on the processor index with
     * `idxs...` */
    virtual std::array<int, DataDim> local_extent(
        std::array<int, GridDim> idxs) = 0;

    /** The total number of elements on the t-th processor */
    int local_element_count(int t) {
        auto extent = local_extent(unflatten<DataDim>(this->grid_, t));
        int count = 1;
        for (auto& length : extent) {
            count *= length;
        }
        return count;
    }

   protected:
    std::array<int, GridDim> grid_;
    std::array<int, DataDim> size_;
};

/**
 * This cyclic partitioning is an initial draft of what will eventually part of
 * a bigger class of partitionings.
 *
 * In particular, block distributions are nearly identical, except for indexing
 * functions being slightly different, make common base class for this.
 *
 * Future challenges include how to coalesce lookups for distributed owner/index
 * tables.
 */
template <int DataDim, int GridDim = DataDim>
class cyclic_partitioning : public partitioning<DataDim, GridDim> {
   public:
    /**
     * Constructs a cyclic partitioning in nD.
     *
     * `grid`: the number of processors in each dimension
     * `data_size`: the global number of processors along each axis
     */
    cyclic_partitioning(std::array<int, GridDim> grid,
                        std::array<int, DataDim> data_size)
        : partitioning<DataDim, GridDim>(grid, data_size) {
        static_assert(GridDim <= DataDim,
                      "Dimensionality of the data should be larger or equal to "
                      "that of the processor grid.");
    }

    /** Compute the local indices of a element using its global indices */
    std::array<int, DataDim> local_index(
        std::array<int, DataDim> index) override final {
        for (int d = 0; d < GridDim; ++d) {
            index[d] = index[d] / this->grid_[d];
        }
        return index;
    }

    /** The total number of elements along each axis on the processor index with
     * `idxs...` */
    std::array<int, DataDim> local_extent(
        std::array<int, GridDim> idxs) override final {
        std::array<int, DataDim> size;
        for (int dim = 0; dim < DataDim; ++dim) {
            size[dim] = (this->size_[dim] + this->grid_[dim] - idxs[dim] - 1) /
                        this->grid_[dim];
        }
        for (int dim = GridDim; dim < DataDim; ++dim) {
            size[dim] *= this->size_[dim];
        }
        return size;
    }

    /** Cyclic in first 'GridDim' dimensions. */
    std::array<int, GridDim> owner(std::array<int, DataDim> xs) override final {
        std::array<int, GridDim> result;
        for (int d = 0; d < GridDim; ++d) {
            result[d] = xs[d] % this->grid_[d];
        }
        return result;
    }
};
/**
 * A block distribution.
 */
template <int DataDim, int GridDim = DataDim>
class block_partitioning : public partitioning<DataDim, GridDim> {
   public:
    /**
     * Constructs a block partitioning in nD.
     *
     * `grid`: the number of processors in each dimension
     * `data_size`: the global number of processors along each axis
     */
    block_partitioning(std::array<int, GridDim> grid,
                       std::array<int, DataDim> data_size)
        : partitioning<DataDim, GridDim>(grid, data_size) {
        static_assert(GridDim <= DataDim,
                      "Dimensionality of the data should be larger or equal to "
                      "that of the processor grid.");
        for (int d = 0; d < GridDim; ++d) {
            block_size_[d] = ((data_size[d] - 1) / grid[d]) + 1;
        }
    }

    /** Compute the local indices of a element using its global indices */
    std::array<int, DataDim> local_index(
        std::array<int, DataDim> index) override final {
        for (int d = 0; d < GridDim; ++d) {
            index[d] = index[d] % block_size_[d];
        }
        return index;
    }

    /** The total number of elements along each axis on the processor index with
     * `idxs...` */
    std::array<int, DataDim> local_extent(
        std::array<int, GridDim> idxs) override final {
        std::array<int, DataDim> size;
        for (int dim = 0; dim < GridDim; ++dim) {
            size[dim] = (this->size_[dim] + this->grid_[dim] - idxs[dim] - 1) /
                        this->grid_[dim];
        }
        for (int dim = GridDim; dim < DataDim; ++dim) {
            size[dim] *= this->size_[dim];
        }
        return size;
    }

    /** Block in first 'GridDim' dimensions. */
    std::array<int, GridDim> owner(std::array<int, DataDim> xs) override final {
        std::array<int, GridDim> result;
        for (int d = 0; d < GridDim; ++d) {
            result[d] = xs[d] / block_size_[d];
        }
        return result;
    }

    // obtain the block size in each dimension
    std::array<int, GridDim> block_size() const { return block_size_; }

    /** Obtain the origin of the block of processor `t`. */
    std::array<int, DataDim> origin(std::array<int, DataDim> multi_index) {
        std::array<int, DataDim> result;
        for (int d = 0; d < DataDim; ++d) {
            result[d] = block_size_[d] * multi_index[d];
        }
        return result;
    }

    /** Obtain the origin of the block of processor `t`. */
    std::array<int, DataDim> origin(int t) {
        auto multi_index = unflatten<DataDim>(this->grid_, t);
        return origin(multi_index);
    }

   private:
    std::array<int, GridDim> block_size_;
};

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
template <int DataDim>
class binary_partitioning : public partitioning<DataDim, 1> {
   public:
    /**
     * Constructs a cyclic partitioning in nD.
     *
     * `grid`: the number of processors in each dimension
     * `data_size`: the global number of processors along each axis
     */
    binary_partitioning(int procs, std::array<int, DataDim> data_size,
                        binary_tree<split>&& splits)
        : partitioning<DataDim, 1>({procs}, data_size),
          splits_(std::move(splits)) {
        // resize extents and origins
        origins_.resize(procs);
        extents_.resize(procs);

        // compute origins and extents here
        for (int proc = 0; proc < procs; ++proc) {
            std::array<int, DataDim> left_bound = {0};
            std::array<int, DataDim> right_bound = this->size_;

            auto node = splits_.root.get();
            int depth = 0;
            while (node) {
                if (proc & (1 << depth)) {
                    left_bound[node->value.d] = node->value.a;
                    node = node->right.get();
                } else {
                    right_bound[node->value.d] = node->value.a;
                    node = node->left.get();
                }
                ++depth;
            }
            origins_[proc] = left_bound;
            for (int d = 0; d < DataDim; ++d) {
                extents_[proc][d] = right_bound[d] - left_bound[d];
            }
        }
    }

    /** Compute the local indices of a element using its global indices */
    std::array<int, DataDim> local_index(
        std::array<int, DataDim> index) override final {
        auto t = owner(index)[0];
        for (int d = 0; d < DataDim; ++d) {
            index[d] -= origins_[t][d];
        }
        return index;
    }

    /** The total number of elements along each axis on the processor index with
     * `idxs...` */
    std::array<int, DataDim> local_extent(
        std::array<int, 1> idxs) override final {
        return extents_[idxs[0]];
    }

    std::array<int, 1> owner(std::array<int, DataDim> xs) override final {
        // we encode the path to the final volume as a bit pattern, that will be
        // the processor id
        auto node = splits_.root.get();
        int proc = 0;
        int depth = 0;
        while (node) {
            if (xs[node->value.d] < node->value.a) {
                node = node->left.get();
            } else {
                proc += 1 << depth;
                node = node->right.get();
            }
            ++depth;
        }
        return {proc};
    }

    std::array<int, DataDim> origin(int t) const { return origins_[t]; }

   private:
    binary_tree<split> splits_;
    std::vector<std::array<int, DataDim>> origins_;
    std::vector<std::array<int, DataDim>> extents_;
};

}  // namespace bulk
