#pragma once

#include <mpi.h>

namespace bulk {
namespace mpi {

class world_provider {
   public:
    world_provider() {
        MPI_Comm_size(MPI_COMM_WORLD, &nprocs_);
        MPI_Comm_rank(MPI_COMM_WORLD, &pid_);
        MPI_Get_processor_name(name_, &name_length_);

        tag_size_ = 0;
    }

    int active_processors() const { return nprocs_; }
    int processor_id() const { return pid_; }

    void sync() const {
        // FIXME: what if spawning with fewer processors than exist
        MPI_Barrier(MPI_COMM_WORLD);
    }

    void internal_put_(int processor, void* value, void* variable, size_t size,
                       int offset, int count) {}

    int register_location_(void* location, size_t size) { return 0; }

    void unregister_location_(void* location) {}

    void internal_get_(int processor, void* variable, void* target, size_t size,
                       int offset, int count) {}

    virtual void internal_send_(int processor, void* tag, void* content,
                                size_t tag_size, size_t content_size) {}

   private:
    size_t tag_size_ = 0;
    char name_[MPI_MAX_PROCESSOR_NAME];
    int name_length_;
    int pid_;
    int nprocs_;
};

}  // namespace mpi
}  // namespace bulk
