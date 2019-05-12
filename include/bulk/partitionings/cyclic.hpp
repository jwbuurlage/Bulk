#include "partitioning.hpp"

namespace bulk {

/**
 * A cyclic partitioning distributes the indices of a D-dimension space over the
 * first G axes, where G is the dimensionality of the processor grid.
 */
template <int D, int G = D>
class cyclic_partitioning : public cartesian_partitioning<D, G> {
  public:
    using cartesian_partitioning<D, G>::owner;
    using cartesian_partitioning<D, G>::local;
    using cartesian_partitioning<D, G>::global;
    using cartesian_partitioning<D, G>::local_size;

    /**
     * Constructs a cyclic partitioning in nD.
     *
     * `grid`: the number of processors in each dimension
     * `data_size`: the global number of processors along each axis
     */
    cyclic_partitioning(index_type<D> data_size, index_type<G> grid)
    : cartesian_partitioning<D, G>(data_size, grid) {
        static_assert(G <= D,
                      "Dimensionality of the data should be larger or equal to "
                      "that of the processor grid.");
    }

    /** Compute the local indices of a element using its global indices */
    index_type<D> local(index_type<D> index) override final {
        for (int d = 0; d < G; ++d) {
            index[d] = index[d] / this->grid_size_[d];
        }
        return index;
    }

    /** Local to global */
    index_type<D> global(index_type<D> xs, index_type<G> processor) override final {
        auto result = xs;
        for (int d = 0; d < G; ++d) {
            result[d] = result[d] * this->grid_size_[d] + processor[d];
        }
        return result;
    }

    /** The total number of elements along each axis on the processor index with
     * `idxs...` */
    index_type<D> local_size(index_type<G> idxs) override final {
        index_type<D> size;
        for (int dim = 0; dim < G; ++dim) {
            size[dim] =
            (this->global_size_[dim] + this->grid_size_[dim] - idxs[dim] - 1) /
            this->grid_size_[dim];
        }
        for (int dim = G; dim < D; ++dim) {
            size[dim] = this->global_size_[dim];
        }
        return size;
    }

    /** Cyclic in first 'G' dimensions. */
    index_type<G> multi_owner(index_type<D> xs) override final {
        index_type<G> result;
        for (int d = 0; d < G; ++d) {
            result[d] = xs[d] % this->grid_size_[d];
        }
        return result;
    }

    virtual int owner(int g, int i) override final {
        return i % this->grid_size_[g];
    }

    virtual int local(int g, int i) override final {
        return i / this->grid_size_[g];
    }

    virtual int global(int g, int u, int i) override final {
        return i * this->grid_size_[g] + u;
    }

    virtual int local_size(int g, int u) override final {
        if (g < G) {
            return (this->global_size_[g] + this->grid_size_[g] - u - 1) /
                   this->grid_size_[g];
        } else {
            return this->global_size_[u];
        }
    }
};

} // namespace bulk
