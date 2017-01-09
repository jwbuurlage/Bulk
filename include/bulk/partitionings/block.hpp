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
    using rectangular_partitioning<D, G>::local_to_global;

    /**
     * Constructs a block partitioning in nD.
     *
     * `grid`: the number of processors in each dimension
     * `data_size`: the global number of processors along each axis
     */
    block_partitioning(bulk::world& world, index_type<D> data_size,
                       index_type<G> grid)
        : rectangular_partitioning<D, G>(world, data_size, grid) {
        static_assert(G <= D,
                      "Dimensionality of the data should be larger or equal to "
                      "that of the processor grid.");
        for (int d = 0; d < G; ++d) {
            block_size_[d] = ((data_size[d] - 1) / grid[d]) + 1;
        }
        for (int d = G; d < D; ++d) {
            block_size_[d] = data_size[d];
        }
    }

    /** Compute the local indices of a element using its global indices */
    index_type<D> global_to_local(index_type<D> index) override final {
        for (int d = 0; d < D; ++d) {
            index[d] = index[d] % block_size_[d];
        }

        return index;
    }

    /** The total number of elements along each axis on the processor index with
     * `idxs...` */
    index_type<D> local_size(index_type<G> idxs) override final {
        index_type<D> size;
        for (int dim = 0; dim < G; ++dim) {
            size[dim] = (this->global_size_[dim] + this->grid_size_[dim] -
                         idxs[dim] - 1) /
                        this->grid_size_[dim];
        }
        for (int dim = G; dim < D; ++dim) {
            size[dim] = this->global_size_[dim];
        }
        return size;
    }

    /** Block in first 'G' dimensions. */
    index_type<G> grid_owner(index_type<D> xs) override final {
        index_type<G> result;
        for (int d = 0; d < G; ++d) {
            result[d] = xs[d] / block_size_[d];
        }
        return result;
    }

    // obtain the block size in each dimension
    index_type<G> block_size() const { return block_size_; }

    /** Obtain the origin of the block of processor `t`. */
    index_type<D> origin(int t) const override {
        auto multi_index = unflatten<G>(this->grid_size_, t);
        index_type<D> result;
        for (int d = 0; d < G; ++d) {
            result[d] = block_size_[d] * multi_index[d];
        }
        return result;
    }

   private:
    index_type<G> block_size_;
};

}  // namespace bulk
