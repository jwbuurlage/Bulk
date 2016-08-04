#pragma once

#include <type_traits>

#include <mpi.h>
#include <boost/bimap.hpp>

namespace bulk {
namespace mpi {

enum class receive_category : int { var_put = 0, var_get = 1, message = 2 };
using receive_type = std::underlying_type<receive_category>::type;

class world_provider {
   public:
    using var_id_type = int;

    struct put_header {
        var_id_type var_id;
        size_t offset;
    };

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

        //// probe for incoming messages

        while (true) {
            MPI_Status status = {};
            int flag = 0;
            MPI_Iprobe(MPI_ANY_SOURCE,
                       static_cast<receive_type>(receive_category::var_put),
                       MPI_COMM_WORLD, &flag, &status);
            if (flag == 0) break;

            int count = 0;
            MPI_Get_count(&status, MPI_BYTE, &count);
            if (count == 0) break;

            // FIXME again terribly unoptimized
            void* buffer = malloc(count);
            MPI_Recv(buffer, count, MPI_BYTE, MPI_ANY_SOURCE,
                     static_cast<receive_type>(receive_category::var_put),
                     MPI_COMM_WORLD, &status);

            put_header header = ((put_header*)buffer)[0];

            memcpy((char*)locations_.right.at(header.var_id) + header.offset,
                   &((put_header*)buffer)[1], count - sizeof(put_header));
            free(buffer);
        }

        MPI_Barrier(MPI_COMM_WORLD);
    }

    void internal_put_(int processor, void* value, void* variable, size_t size,
                       int offset, int count) {
        /* FIXME: we really dont want to do it like this:
         * - dynamic allocation
         * - type erasures
         * move to templated internal put and use proper code
         * */
        put_header header = {};
        header.var_id = locations_.left.at(variable);
        header.offset = offset * size;

        auto data_size = sizeof(put_header) + size * count;
        void* payload = malloc(data_size);
        ((put_header*)payload)[0] = header;

        memcpy((&((put_header*)payload)[1]), value, size * count);

        MPI_Send(payload, data_size, MPI_BYTE, processor,
                 static_cast<receive_type>(receive_category::var_put),
                 MPI_COMM_WORLD);

        free(payload);
    }

    int register_location_(void* location, size_t size) {
        locations_.insert(bimap_pair(location, vars_));
        return vars_++;
    }

    void unregister_location_(void* location) {
        locations_.left.erase(location);
    }

    void internal_get_(int processor, void* variable, void* target, size_t size,
                       int offset, int count) {}

    virtual void internal_send_(int processor, void* tag, void* content,
                                size_t tag_size, size_t content_size) {}

    std::string name() { return std::string(name_); }

   private:
    size_t tag_size_ = 0;
    char name_[MPI_MAX_PROCESSOR_NAME];
    int name_length_;
    int pid_;
    int nprocs_;
    int vars_ = 0;

    boost::bimap<void*, var_id_type> locations_;
    using bimap_pair = decltype(locations_)::value_type;
};

}  // namespace mpi
}  // namespace bulk
