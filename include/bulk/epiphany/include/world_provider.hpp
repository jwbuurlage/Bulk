#pragma once
#include <cstdint>
#include "epiphany_internals.hpp"

namespace bulk {
namespace epiphany {

class world_provider {
  public:
    world_provider();
    ~world_provider();

    int active_processors() const { return nprocs_; }
    int processor_id() const { return local_pid_; }

    void sync();

    /// Barrier that not resolve outstanding communication like sync.
    void barrier();

    var_id_t register_location_(void* location, size_t size) {
        var_id_t id = -1;
        for (var_id_t i = 0; i < MAX_VARS; ++i) {
            if (var_list_[i] == 0) {
                var_list_[i] = transform_address_(location, local_pid_);
                id = i;
                break;
            }
        }
        barrier();
        if (id == -1) {
            // TODO: error message and return code
            return 0;
        }
        return id;
    }

    void move_location_(int id, void* newlocation) {
        var_list_[id] = newlocation;
        barrier();
    }

    void unregister_location_(void* location) {
        for (var_id_t i = 0; i < MAX_VARS; ++i) {
            if (var_list_[i] == location) {
                var_list_[i] = 0;
                break;
            }
        }
    }

    // This gets an int from `world` so do not use `var_id_t` here.
    void unregister_location_(int id) { var_list_[id] = 0; }

    // TODO
    void internal_put_(int processor, void* value, void* variable, size_t size,
                       int offset, int count) {
        return;
    }

    // TODO
    void internal_get_(int processor, void* variable, void* target, size_t size,
                       int offset, int count) {
        return;
    }

    // TODO
    void internal_send_(int processor, void* tag, void* content,
                        size_t tag_size, size_t content_size) {
        return;
    }

    void* get_direct_address_(int pid, int id) {
        void** var_list_remote =
            (void**)transform_address_((void*)var_list_, pid);
        // the remote var list already contains global versions of addresses
        return var_list_remote[id];
    }

  private:
    // Transform local address to global address,
    // unless it already is global
    void* transform_address_(void* addr, int pid) const {
        if ((unsigned(addr) & 0xfff00000) == 0)
            return (void*)(unsigned(addr) | (uint32_t(coreids_[pid]) << 20));
        return addr;
    }

    void write_syncstate_(int8_t state);

    // This is our own version of e_barrier_init which is shorter
    void barrier_init_();

    // ARM core will set this, epiphany will poll this
    volatile int8_t syncstate_;

    pid_t local_pid_;
    pid_t nprocs_;
    uint16_t coreids_[NPROCS]; // pid to coreid mapping

    void* var_list_[MAX_VARS]; // only contains GLOBAL pointers

    // sync barrier
    volatile int8_t sync_barrier_[NPROCS];
    volatile int8_t* sync_barrier_tgt_[NPROCS];

    // Mutex for print
    e_mutex_t print_mutex_;
    friend void print(const char* format, ...);

    // Mutex for ext_malloc (internal malloc does not have mutex)
    // e_mutex_t malloc_mutex_;
};

} // namespace epiphany
} // namespace bulk

