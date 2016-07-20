#pragma once
#include <cstdint>
#include <cstddef>
#include "epiphany_internals.hpp"

namespace bulk {
namespace epiphany {

// There can be at most NPROCS mutexes,
// because each core stores exactly one.
enum MUTEXID : int {
        MUTEX_PRINT = 0,
        MUTEX_EXTMALLOC = 1,
        MUTEX_STREAM = 2
    };


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
        var_list_[id] = transform_address_(newlocation, local_pid_);
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
            (void**)transform_address_local_((void*)var_list_, pid);
        // the remote var list already contains global versions of addresses
        return var_list_remote[id];
    }

    // Optimized version of `e_mutex_lock`
    void mutex_lock_(MUTEXID mutex_id) {
        int* pmutex = (int*)transform_address_local_(&mutexes_, mutex_id);
        uint32_t coreid = coreids_[local_pid_];
        uint32_t offset = 0;
        uint32_t val;
        do {
            val = coreid;
            __asm__ __volatile__("testset	%[val], [%[pmutex], %[offset]]"
                                 : [val] "+r"(val)
                                 : [pmutex] "r"(pmutex), [offset] "r"(offset)
                                 : "memory");
        } while (val != 0);
    }

    void mutex_unlock_(MUTEXID mutex_id) {
        const register uint32_t zero = 0;
        int* pmutex = (int*)transform_address_local_(&mutexes_, mutex_id);
        __asm__ __volatile__("str %[zero], [%[pmutex]]"
                             : /* no outputs */
                             : [zero] "r"(zero), [pmutex] "r"(pmutex)
                             : "memory");
        return;
    }

  private:
    // Transform local address to global address,
    // unless it already is global
    void* transform_address_(void* addr, int pid) const {
        if ((unsigned(addr) & 0xfff00000) == 0)
            return (void*)(unsigned(addr) | (uint32_t(coreids_[pid]) << 20));
        return addr;
    }

    // Same as above but assumes address is local to save one check
    void* transform_address_local_(void* addr, int pid) const {
        return (void*)(unsigned(addr) | (uint32_t(coreids_[pid]) << 20));
    }

    void write_syncstate_(int8_t state) {
        syncstate_ = state;                    // local variable
        combuf_->syncstate[local_pid_] = state; // being polled by ARM
    }

    // This is our own version of e_barrier_init which is shorter
    void barrier_init_();

    // print needs access to syncstate_
    friend void print(const char* format, ...);

    // ARM core will set this, epiphany will poll this
    volatile int8_t syncstate_;

    pid_t local_pid_;
    pid_t nprocs_;
    uint16_t coreids_[NPROCS]; // pid to coreid mapping

    void* var_list_[MAX_VARS]; // only contains GLOBAL pointers

    // sync barrier
    volatile int8_t sync_barrier_[NPROCS];
    volatile int8_t* sync_barrier_tgt_[NPROCS];

    // Every core has a copy, all are different mutexes.
    int mutexes_;
};

} // namespace epiphany
} // namespace bulk

