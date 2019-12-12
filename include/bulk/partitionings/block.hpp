#include "partitioning.hpp"

namespace bulk {

/**
 * A block distribution. This equally block-distributes the first G axes.
 */
template <int D, int G = D>
class block_partitioning : public rectangular_partitioning<D, G> {
  public:
    using rectangular_partitioning<D, G>::local_size;
    using rectangular_partitioning<D, G>::origin;
    using rectangular_partitioning<D, G>::global;

    /**
     * Constructs a block partitioning in nD.
     *
     * `grid`: the number of processors in each dimension
     * `data_size`: the global number of processors along each axis
     */
    block_partitioning(index_type<D> data_size, index_type<G> grid)
    : block_partitioning(data_size, grid, iota_()) {}

    /**
     * Constructs a block partitioning in nD.
     *
     * `grid`: the number of processors in each dimension
     * `data_size`: the global number of processors along each axis
     * `axes`: an array of size `G` that indicates the axes over which to
     * partition
     */
    block_partitioning(index_type<D> data_size, index_type<G> grid, index_type<G> axes)
    : rectangular_partitioning<D, G>(data_size, grid), axes_(axes) {
        static_assert(G <= D,
                      "Dimensionality of the data should be larger or equal to "
                      "that of the processor grid.");
        block_size_ = data_size;

        for (int i = 0; i < G; ++i) {
            int d = axes_[i];
            block_size_[d] = ((data_size[d] - 1) / grid[i]) + 1;
        }
    }

    /** Compute the local indices of a element using its global indices */
    index_type<D> local(index_type<D> index) override final {
        auto t = this->owner(index);
        for (int d = 0; d < D; ++d) {
            index[d] = index[d] - this->origin(t)[d];
        }
        return index;
    }

    /** The total number of elements along each axis on the processor index with
     * `idxs...` */
    index_type<D> local_size(index_type<G> idxs) override final {
        index_type<D> size = this->global_size_;
        for (int i = 0; i < G; ++i) {
            auto dim = axes_[i];

            size[dim] = (this->global_size_[dim] + this->grid_size_[i] - idxs[i] - 1) /
                        this->grid_size_[i];
        }
        return size;
    }

    /** Block in first 'G' dimensions. */
    index_type<G> multi_owner(index_type<D> xs) override final {
        index_type<G> result = {};
        for (int i = 0; i < G; ++i) {
            auto d = this->axes_[i];
            auto n = this->global_size_[d];
            auto k = this->block_size_[d];
            auto P = this->grid_size_[i];
            // For irregular block partitionings, only the first 'l' processors will
            // have block_size_, the other n - l processors have block_size - 1.
            auto l = P - (k * P - n);
            if (xs[d] <= k * l) {
                result[i] = xs[d] / block_size_[d];
            } else {
                result[i] = l + (xs[d] - k * l) / (block_size_[d] - 1);
            }
        }
        return result;
    }

    /** Obtain the block size in each dimension. */
    index_type<D> block_size() const { return block_size_; }

    /** Obtain the origin of the block of processor `multi_index'. */
    index_type<D> origin(index_type<G> multi_index) const override {
        index_type<D> result = {};
        for (int i = 0; i < G; ++i) {
            auto d = axes_[i];
            auto n = this->global_size_[d];
            auto k = this->block_size_[d];
            auto P = this->grid_size_[i];
            auto l = P - (k * P - n);
            auto t = multi_index[i];
            if (t <= l) {
                result[d] = k * multi_index[i];
            } else {
                result[d] = k * l + (t - l) * (k - 1);
            }
        }
        return result;
    }

  private:
    index_type<D> block_size_;
    index_type<G> axes_;

    static index_type<G> iota_() {
        index_type<G> result = {};
        for (int i = 0; i < G; ++i) {
            result[i] = i;
        }
        return result;
    }
};

} // namespace bulk
