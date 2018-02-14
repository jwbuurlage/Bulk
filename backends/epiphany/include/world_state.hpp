#pragma once
#include <cstdint>
#include <cstddef>
#include <utility> // for std::pair
#include <vector>
#include "epiphany_internals.hpp"

namespace bulk {

// Taken from `thread` backend
struct registered_variable {
    registered_variable() : base(0), buffer(0), receiveBuffer(0), size(0) {}
    ~registered_variable() {}

    class var_base* base;
    void* buffer;        // Local copy, stored in var::impl
    char* receiveBuffer; // Fixed size receive buffer, allocated by world
    size_t capacity;     // Size of the allocated receive buffer, can change
    size_t size;         // Current filling of receiveBuffer
    int do_put;          // Flag to signal that data was received
};

typedef std::pair<char*, size_t> sized_buffer;

struct registered_queue {
    registered_queue() : base(0), mutex(0) {}
    ~registered_queue() {}

    class queue_base* base; // Local queue, stored in queue::impl
    std::vector<sized_buffer> receiveBuffers; // Dynamic sized receive buffer
    int mutex;              // Mutex on receiveBuffers
};


namespace epiphany {

// There can be at most NPROCS mutexes,
// because each core stores exactly one.
enum MUTEXID : int {
        MUTEX_PRINT = 0,
        MUTEX_EXTMALLOC = 1,
        MUTEX_STREAM = 2
    };


class world_state {
  public:
    world_state();
    ~world_state();

    int active_processors() const { return nprocs_; }
    int processor_id() const { return local_pid_; }

    void sync();

    /// Barrier that not resolve outstanding communication like sync.
    void barrier();

    void abort() {
        write_syncstate_(SYNCSTATE::ABORT);
        for (;;)
            barrier();
    }

    var_id_t register_location_(class var_base* varbase, void* location, size_t size) {
        var_id_t id = VAR_INVALID;
        for (var_id_t i = 0; i < MAX_VARS; ++i) {
            if (var_list_[i].base == 0) {
                var_list_[i].base = (var_base*)transform_address_(varbase, local_pid_);
                var_list_[i].buffer = location;
                var_list_[i].receiveBuffer = new char[size];
                var_list_[i].capacity = size;
                var_list_[i].size = 0;
                var_list_[i].do_put = 0;
                id = i;
                break;
            }
        }
        barrier();
        if (id == VAR_INVALID) {
            // TODO: error message and return code
            return 0;
        }
        return id;
    }

    // This gets an int from `world` so do not use `var_id_t` here.
    void unregister_location_(int id) {
        auto& v = var_list_[id];
        if (v.receiveBuffer)
            delete[] v.receiveBuffer;
        v.base = 0;
        v.buffer = 0;
        v.receiveBuffer = 0;
        v.capacity = 0;
        v.size = 0;
        return;
    }

    registered_variable& get_var_(int pid, int id) {
        registered_variable* var_list_remote =
            (registered_variable*)transform_address_local_((void*)var_list_, pid);
        // the remote var list already contains global versions of addresses
        return var_list_remote[id];
    }

    int register_queue_(class queue_base* q) {
        var_id_t id = VAR_INVALID;
        for (var_id_t i = 0; i < MAX_VARS; ++i) {
            if (queue_list_[i].base == 0) {
                queue_list_[i].base = q;
                queue_list_[i].receiveBuffers.clear();
                queue_list_[i].mutex = 0;
                id = i;
                break;
            }
        }
        barrier();
        if (id == VAR_INVALID) {
            // TODO: error message and return code
            return 0;
        }
        return id;
    }

    registered_queue& get_queue_(int id, int pid) const {
        registered_queue* queue_list_remote =
            (registered_queue*)transform_address_local_((void*)queue_list_,
                                                        pid);
        return queue_list_remote[id];
    }

    void unregister_queue_(int id) {
        auto& q = queue_list_[id];
        q.base = 0;
        for (auto& p : q.receiveBuffers) {
            delete[] p.first;
        }
        q.receiveBuffers.clear();
        q.mutex = 0;
        return;
    }

    // Optimized version of `e_mutex_lock`
    void mutex_lock_(int* global_addr) {
        int* pmutex = global_addr;
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

    void mutex_unlock_(int* global_addr) {
        const uint32_t zero = 0;
        int* pmutex = global_addr;
        __asm__ __volatile__("str %[zero], [%[pmutex]]"
                             : /* no outputs */
                             : [zero] "r"(zero), [pmutex] "r"(pmutex)
                             : "memory");
        return;
    }

    void mutex_lock_(MUTEXID mutex_id) {
        mutex_lock_((int*)transform_address_local_(&mutexes_, mutex_id));
    }

    void mutex_unlock_(MUTEXID mutex_id) {
        mutex_unlock_((int*)transform_address_local_(&mutexes_, mutex_id));
    }

  private:
    //
    // Private functions
    //

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
    friend class stream;

    // This is our own version of e_barrier_init which is shorter
    void barrier_init_();

    // world_provider deconstructor and print
    // need access to syncstate_
    friend void print(const char* format, ...);
    friend class world_provider;

    //
    // Private variables
    // The single byte variables are grouped at the end to avoid padding
    //

    // This is no longer feasible when the number of cores goes up
    uint16_t coreids_[NPROCS]; // pid to coreid mapping

    registered_variable var_list_[MAX_VARS]; // only contains GLOBAL pointers
    registered_queue  queue_list_[MAX_VARS]; // only contains GLOBAL pointers

    // sync barrier
    // This method is no longer feasible when number of cores goes up, although
    // this method is taken from the ESDK so they will probably provide a new
    // one anyway
    volatile int8_t sync_barrier_[NPROCS];
    volatile int8_t* sync_barrier_tgt_[NPROCS];

    // Every core has a copy, all are different mutexes.
    int mutexes_;

    // ARM core will set this, epiphany will poll this
    volatile int8_t syncstate_;

    pid_t local_pid_;
    pid_t nprocs_;
};

extern world_state state;

} // namespace epiphany
} // namespace bulk

