#pragma once
#include <cstdint>
#include <cstring>
#include <functional>
#include <iostream>
#include <string>
#include <vector>
#include <utility>

extern "C" {
#include <e-hal.h>
#include <e-loader.h>
}

#include "combuf.hpp"

// Include the epiphany side headers to make the lambda-kernels compile
#include "../epiphany.hpp"

namespace bulk {
namespace epiphany {

class environment : public bulk::environment {
  public:
    class stream {
      public:
        // Fill buffer in extmem with data from host
        void fill_stream() {
            // TODO
            int ret = read(buffer, descriptor->offset, capacity);
            if (ret > capacity) {
                std::cerr << "ERROR: Written out of bounds to stream.\n";
            } else if (ret < -1) {
                std::cerr
                    << "ERROR: Invalid return value at stream callback.\n";
            } else if (ret == -1) {
                // Stream finished
                std::cerr << "WARNING: Untested feature 'stream finished'.\n";
                descriptor->filled_size = -1;
            } else if (ret == 0) {
                // No data available right now, but might be later
                std::cerr << "WARNING: Untested feature: no data written to "
                             "stream by host.\n";
                descriptor->filled_size = 0;
            } else {
                // Data has been copied to buffer, usable by Epiphany
                descriptor->filled_size = ret;
            }
        }

        // Flush extmem buffer to host
        void flush_stream() {
            // TODO
            write(buffer, descriptor->offset, requested_capacity);
        }

        // Allocated buffer
        void* buffer; // points to external memory, host address space
        int capacity; // amount of allocated external memory
        int requested_capacity; // can differ because rounding up to 8-multiple
        // Pointer to descriptor in extmem (accessible by Epiphany)
        stream_descriptor* descriptor;

        /**
         * Read callback for stream
         * This function may be called by the environment anytime between
         * the creation of the stream and the completion of `ebsp_spmd`.
         * - void* buffer
         *         A pointer to the buffer in external memory where the data
         *         can be written, corresponding to `offset` in the stream.
         * - int offset
         *         The offset into the stream at which data is needed,
         *         this is already incorporated into `buffer`.
         * - int size_requested
         *         The amount of bytes requested.
         * - return int size_written
         *         The amount of bytes written to the stream by the function.
         *         This MUST be at most `size_requested`.
         *         The function can return `-1` to indicate the end of the
         *         stream.
         *         The function can return `0` to indicate no data is
         *         available right now but might be later.
         */
        std::function<int(void*, uint32_t, uint32_t)> read;

        /**
         * Write callback for stream
         * This function may be called by the environment anytime during
         * the call to `ebsp_spmd`.
         * The function can be left emtpy if no data from the kernels is needed.
         * - const void* buffer
         *         A pointer to the buffer in external memory where the data
         *         written by the kernel is available.
         * - int offset
         *         The offset into the stream at which data is written.
         * - int bytes_written
         *         The amount of bytes written by the kernel.
         * - return void
         */
        std::function<void(const void*, uint32_t, uint32_t)> write;
    };

    environment() {
        env_initialized_ = 0;
        initialize_();
    }
    ~environment() { finalize_(); }

    /// Check if the hub is properly initialized and ready to run
    /// an Epiphany program
    bool is_valid() const { return env_initialized_ >= 2; }

    void spawn(int processors, const char* image_name);

#ifdef __clang__
    // Actually we would prefer to have a raw function pointer instead of an
    // std::function object because this makes sure that captures are
    // not allowed.
    // However, using a raw object will make generic lambdas
    // (ones that use auto& for the first argument) not compile anymore.
    // Therefore, the std::function is used instead of:
    // void spawn(int processors,
    //           void (*kernelfunc)(bulk::world<backend>&, int, int));
    // Note that spawn is never implemented since the library is never compiled
    // with clang so the linking stage is never reached.
    void spawn(int processors,
               std::function<void(bulk::world&, int, int)> f);
#else
    // When not compiling with the tool, there should be an error
    // when the user tries to use the function, but NO error when the
    // function is NOT called.
    // This trick is taken from
    // http://stackoverflow.com/questions/34745581/forbids-functions-with-static-assert
    template <typename...>
    struct always_false {
        static constexpr bool value = false;
    };
    template <typename T>
    void spawn(int processors, T f) {
        static_assert(always_false<T>::value,
                      "Use bulk-compile-tool to compile lambda kernels");
    }

    // This function is used when the kernel binary is embedded in the host
    void spawn(int processors,
               std::pair<unsigned char*, unsigned char*> file_start_end);
#endif

    int available_processors() const override final { return nprocs_available_; }

    void set_log_callback(std::function<void(int, const std::string&)> f) override final {
        log_callback_ = f;
    }

    /**
     * Create a new stream that any processor can open.
     *
     * @param read Called when more data is requested by the kernel
     * @param write Called when data was written by the kernel
     * @param capacity Size of external memory buffer
     * @return true if stream was succesfully created
     *
     * See class `environment::stream` for description of callbacks.
     *
     * Capacity must be nonzero and will be rounded up to the nearest multiple
     * of 8.
     *
     * The total size of the data can be bigger, and does not have to
     * fit in this buffer at once.
     *
     * `read` will be called when more data is required by the system.
     *
     * `read` and `write` may be called by the environment anytime between
     * the creation of the stream and the completion of `ebsp_spmd`.
     */
    bool
    create_stream(std::function<uint32_t(void*, uint32_t, uint32_t)> read,
                  std::function<void(const void*, uint32_t, uint32_t)> write,
                  uint32_t capacity) {
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
        if (read == nullptr || write == nullptr) {
            std::cerr << "ERROR: Must provide valid read and write functions "
                         "to the stream.\n";
            return false;
        }
        uint32_t rounded_capacity = ((capacity + 7) / 8) * 8; // round up
        void* buffer = ext_malloc_(rounded_capacity);
        if (buffer == 0) {
            std::cerr << "ERROR: Stream capacity " << capacity
                      << " does not fit in external memory.\n";
            return false;
        }
        streams.push_back(stream());
        stream& s = streams.back();
        s.buffer = buffer;
        s.capacity = rounded_capacity;
        s.requested_capacity = capacity;
        s.descriptor = 0;
        s.read = read;
        s.write = write;
        // TODO: start requesting data?
        return true;
    }

    /**
     * Create a new stream that any processor can open.
     *
     * @param data Buffer holding the initial data for the stream
     * @param data_size Size of the data
     * @param capacity Size of external memory buffer
     * @return true if stream was succesfully created
     *
     * `capacity` must be nonzero and will be rounded up to the nearest multiple
     * of 8.
     *
     * `data_size` must be large enough to hold any data that the kernel
     * will write to the stream.
     *
     * The total size of the data (`data_size`) can be bigger than the
     * size of the external memory buffer (`capacity`).
     */
    bool create_stream(void* data, uint32_t data_size, uint32_t capacity) {
        int stream_id = streams.size();
        return create_stream(
            [data, data_size](void* dst, uint32_t offset,
                              uint32_t size_requested) -> int {
                // Kernel wants to read data
                if (size_requested > data_size - offset)
                    size_requested = data_size - offset;
                if (size_requested <= 0)
                    return -1;
                ::memcpy(dst, (void*)(unsigned(data) + offset), size_requested);
                return size_requested;
            },
            [data, data_size, stream_id](const void* buf, uint32_t offset,
                                         uint32_t bytes_written) {
                // Kernel has written data
                if (offset + bytes_written > data_size) {
                    std::cerr
                        << "WARNING: Kernel is writing out of bounds on stream "
                        << stream_id << '\n';
                    if (offset > data_size)
                        return;
                    // Still write the part that fits
                    bytes_written = data_size - offset;
                }
                ::memcpy((void*)(unsigned(data) + offset), buf, bytes_written);
                return;
            },
            capacity);
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
    combuf* host_combuf_;
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
        return (void*)((unsigned)ptr - (unsigned)host_combuf_ + E_COMBUF_ADDR);
    }

    void* e_to_host_pointer_(void* ptr) {
        return (void*)((unsigned)ptr - E_COMBUF_ADDR + (unsigned)host_combuf_);
    }
};

} // namespace epiphany
} // namespace bulk
