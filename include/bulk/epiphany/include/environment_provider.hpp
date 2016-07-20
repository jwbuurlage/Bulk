#pragma once
#include <iostream>
#include <string>
#include <cstdint>
#include <cstring>
#include <functional>
#include <vector>

extern "C" {
#include <e-hal.h>
#include <e-loader.h>
}

#include "combuf.hpp"

namespace bulk {
namespace epiphany {

class provider {
  public:
    class stream {
      public:
        void fill_stream() {
            int ret = callback(buffer, descriptor->offset, capacity);
            if (ret > capacity) {
                std::cerr << "ERROR: Written out of bounds to stream.\n";
            } else if (ret < -1) {
                std::cerr
                    << "ERROR: Invalid return value at stream callback.\n";
            } else if (ret == -1) {
                // Stream finished
                descriptor->size = -1;
            } else if (ret == 0) {
                // No data available right now, but might be later
                descriptor->size = 0;
            } else {
                // Data has been copied to buffer, usable by Epiphany
                descriptor->size = ret;
            }
        }

        // Allocated buffer
        void* buffer; // points to external memory, host address space
        int capacity; // amount of allocated external memory
        // Pointer to descriptor, accessible by Epiphany
        stream_descriptor* descriptor;

        // Callback to get more stream data
        // This function may be called by the environment anytime between
        // the creation of the stream and the completion of `ebsp_spmd`.
        // * void* buffer
        //         A pointer to the buffer in external memory where the data
        //         can be written, corresponding to `offset` in the stream.
        // * int offset
        //         The offset into the stream at which data is needed.
        // * int size_requested
        //         The amount of bytes requested.
        // * return int size_written
        //         The amount of bytes written to the stream by the function.
        //         This MUST be at most `size_requested`.
        //         The function can return `-1` to indicate the end of the
        //         stream.
        //         The function can return `0` to indicate no data is
        //         available right now but might be later.
        std::function<int(void*, int, int)> callback;
    };

    provider() {
        env_initialized_ = 0;
        initialize_();
    }
    ~provider() { finalize_(); }

    /// Check if the hub is properly initialized and ready to run
    /// an Epiphany program
    bool is_valid() const { return env_initialized_ >= 2; }

    void spawn(int processors, const char* image_name );

    int available_processors() const { return nprocs_available_; }

    void setLogCallback(std::function<void(int, const std::string&)> f) {
        log_callback_ = f;
    }

    /*
     * Create a new stream that any processor can open
     *
     * The capacity is the total size allocated for the stream.
     * Capacity must be nonzero.
     * Capacity will be rounded up to the nearest multiple of 8.
     * The total size of the stream can be bigger, and does not have to
     * fit in this buffer at once.
     *
     * The function passed will be called when more data is required by
     * the system. See `stream` above for function description.
     * Returns true if a stream was succesfully created.
     *
     * The function may be called by the environment anytime between
     * the creation of the stream and the completion of `ebsp_spmd`.
     */
    bool create_stream(uint32_t capacity,
                       std::function<int(void*, int, int)> f) {
        // Check if environment is initialized, but Epiphany is not running yet
        if (env_initialized_ != 2) {
            std::cerr
                << "ERROR: create_stream called on a hub that was not properly "
                   "initialized.\n";
            return false;
        }
        if (capacity == 0) {
            std::cerr << "ERROR: Stream capacity must be nonzero.\n";
            return false;
        }
        if (f == nullptr) {
            std::cerr
                << "ERROR: Must provide a valid function to the stream.\n";
            return false;
        }
        capacity = ((capacity + 7) / 8) * 8; // round up
        void* buffer = ext_malloc_(capacity);
        if (buffer == 0) {
            std::cerr << "ERROR: Stream capacity " << capacity
                      << " does not fit in external memory.\n";
            return false;
        }
        stream s;
        s.buffer = buffer;
        s.capacity = capacity;
        s.descriptor = 0;
        s.callback = f;
        streams.push_back(s);
        // TODO: start requesting data?
        return true;
    }

  private:
    // Zero if not properly initialized
    // Epiphany system is only available if this value is at least 2
    int env_initialized_;

    // Number of processors available on the system
    int nprocs_available_;

    // Number of processors in use
    // Since it is a square it might not equal rows * cols
    int nprocs_used_;
    int rows_;
    int cols_;

    // The directory of the program and e-program
    // including a trailing slash
    std::string e_directory_;
    // The name of the e-program
    std::string e_fullpath_;

    // Epiphany specific variables
    e_platform_t platform_;
    e_epiphany_t dev_;
    e_mem_t emem_; // Describes mmap info for external memory

    // Points directly to external memory using memory-mapping
    combuf* combuf_;
    void* malloc_base_;

    // Timer storage
    struct timespec ts_start_, ts_end_;

    // Streams
    std::vector<stream> streams;

    // Logging
    std::function<void(int, const std::string&)> log_callback_;

    void initialize_();

    void finalize_();

    void set_core_syncstate_(int pid, SYNCSTATE state);

    // Get the directory that the application is running in
    // and store it in e_directory_ including the trailing slash
    void init_application_path_();

    void update_remote_timer_();

    void microsleep_(int microseconds);

    // External memory malloc system
    void ext_malloc_init_();
    void* ext_malloc_(uint32_t);
    void ext_free_(void*);

    void* host_to_e_pointer_(void* ptr) {
        return (void*)((unsigned)ptr - (unsigned)combuf_ + E_COMBUF_ADDR);
    }

    void* e_to_host_pointer_(void* ptr) {
        return (void*)((unsigned)ptr - E_COMBUF_ADDR + (unsigned)combuf_);
    }
};

} // namespace epiphany
} // namespace bulk
