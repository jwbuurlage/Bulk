#include <environment_provider.hpp>
#include <cstdlib>
#include <cstdio>

// #define __USE_XOPEN2K // already defined in one of the C++ headers
#include <unistd.h> // For `access` and `readlink`
// #define __USE_XOPEN2K
#define __USE_POSIX199309 1
#include <time.h> // clock_gettime and clock_nanosleep

namespace bulk {
namespace epiphany {

void provider::spawn(int processors,
                     std::pair<unsigned char*, unsigned char*> file_buffer) {
    // Create temporary file with kernel as contents
    // Warning: ugly C code ahead
    char filename[32];
    strncpy(filename, "/tmp/bulk-XXXXXX", 32);
    int filedescriptor = mkstemp(filename);
    if (filedescriptor == -1) {
        std::cerr << "ERROR: Could not create temporary file for kernel.\n";
        return;
    }

    size_t file_size = size_t(file_buffer.second - file_buffer.first);
    size_t ret = write(filedescriptor, file_buffer.first, file_size);
    if (ret != file_size) {
        std::cerr << "ERROR: Could not write to temporary file for kernel.\n";
    } else {
        spawn(processors, (const char*)filename);
    }
    unlink(filename);
    close(filedescriptor);
}

void provider::spawn(int processors, const char* image_name) {
    if (!is_valid()) {
        std::cerr << "ERROR: spawn called on hub that was not properly "
                     "initialized."
                  << std::endl;
        return;
    }

    nprocs_used_ = processors;

    if (processors < 1 || processors > NPROCS) {
        std::cerr << "ERROR: spawn called with processors = " << processors
                  << std::endl;
    }

    // First check if the file exists relative to working directory
    if (access(image_name, R_OK) != -1) {
        e_fullpath_ = image_name;
    } else {
        // Now check if it exits relative to host executable
        e_fullpath_ = e_directory_;
        e_fullpath_.append(image_name);

        // Check if the file exists
        if (access(e_fullpath_.c_str(), R_OK) == -1) {
            std::cerr << "ERROR: Could not find epiphany executable: "
                      << e_fullpath_ << std::endl;
            return;
        }
    }

    // Reset core registers to defaults
    if (e_reset_group(&dev_) != E_OK) {
        std::cerr << "ERROR: Could not reset workgroup.\n";
        return;
    }

    // Load the kernel code to core memory
    if (e_load_group(e_fullpath_.c_str(), &dev_, 0, 0, rows_, cols_, E_FALSE) !=
        E_OK) {
        std::cerr << "ERROR: Could not load program to chip." << std::endl;
        return;
    }

    host_combuf_->nprocs = nprocs_used_;
    for (int i = 0; i < NPROCS; i++)
        host_combuf_->syncstate[i] = SYNCSTATE::INIT;

    // Write stream descriptors
    stream_descriptor* descriptors = (stream_descriptor*)ext_malloc_(
        streams.size() * sizeof(stream_descriptor));
    if (descriptors == 0) {
        std::cerr
            << "ERROR: Not enough external memory to write stream descriptors."
            << std::endl;
        return;
    }
    host_combuf_->nstreams = streams.size();
    host_combuf_->streams = (stream_descriptor*)host_to_e_pointer_(descriptors);
    for (size_t i = 0; i < streams.size(); ++i) {
        streams[i].descriptor = &(descriptors[i]);
        descriptors[i].buffer = host_to_e_pointer_(streams[i].buffer);
        descriptors[i].capacity = streams[i].capacity;
        descriptors[i].offset = 0;
        descriptors[i].filled_size = 0;
        descriptors[i].pid = -1;
    }

    // Load streams with data
    // TODO
    // Note that we have to do this before launching the kernels
    // because the kernels will try to open them and the descriptors
    // have to contain a valid `filled_size` at that point.
    // OR: we only do `fill_stream` when the kernel requested it?
    // Note that we can not fill it while a kernel has it open
    // because of race conditions on the stream descriptor.
    for (auto& s : streams)
        s.fill_stream();

    // Starting time
    clock_gettime(CLOCK_MONOTONIC, &ts_start_);
    update_remote_timer_();

    // Start the program by sending SYNC interrupts
    if (e_start_group(&dev_) != E_OK) {
        std::cerr << "ERROR: e_start_group() failed." << std::endl;
        return;
    }

    env_initialized_ = 3;

#ifdef DEBUG
    int iter = 0;
    std::cerr << "Bulk DEBUG: Epiphany cores started.\n";
#endif

    // Main program loop
    int extmem_corrupted = 0;
    for (;;) {
        update_remote_timer_();
        microsleep_(1); // 1000 is 1 millisecond

        // Check sync states
        int counters[SYNCSTATE::COUNT] = {0};

        for (int i = 0; i < NPROCS; i++) {
            SYNCSTATE s = (SYNCSTATE)host_combuf_->syncstate[i];
            if (s >= 0 && s < SYNCSTATE::COUNT) {
                counters[s]++;
            } else {
                extmem_corrupted++;
                if (extmem_corrupted <= 32) { // to avoid overflow
                    std::cerr << "ERROR: External memory corrupted.";
                    std::cerr << " syncstate[" << i << "] = " << s << std::endl;
                }
            }

            if (s == SYNCSTATE::MESSAGE) {
                std::string msg(host_combuf_->msgbuf);
                // Reset flag to let epiphany core continue
                set_core_syncstate_(i, SYNCSTATE::CONTINUE);
                // Print message
                if (log_callback_) {
                    log_callback_(i, msg);
                } else {
                    printf("$%02d: %s\n", i, msg.c_str());
                    fflush(stdout);
                }
            }

            if (s == SYNCSTATE::STREAMREQ) {
                // TODO
                std::cerr << "WARNING: Core " << i << " requests stream data "
                                                      "but feature is not "
                                                      "implemented yet.\n";
                set_core_syncstate_(i, SYNCSTATE::CONTINUE);
            }
            if (s == SYNCSTATE::STREAMWRITE) {
                // TODO
                std::cerr << "WARNING: Core " << i << " has written data to "
                                                      "stream but feature is "
                                                      "not implemented yet.\n";
                set_core_syncstate_(i, SYNCSTATE::CONTINUE);
            }
        }

        if (counters[SYNCSTATE::SYNC] == nprocs_used_) {
            std::cout << "(BSP) Host sync. Not implemented." << std::endl;
            for (int i = 0; i < nprocs_used_; i++)
                set_core_syncstate_(i, SYNCSTATE::CONTINUE);
        }

        if (counters[SYNCSTATE::ABORT]) {
            std::cout << "(BSP) ERROR: spmd program aborted." << std::endl;
            // Abort all cores because they are in an unusable state
            // and set proper environment state
            e_reset_system();
            finalize_();
            return;
        }
        if (counters[SYNCSTATE::FINISH] == nprocs_used_)
            break;

#ifdef DEBUG
        if (iter % 1000 == 0) {
            std::cerr << "Core bulk states:";
            for (int i = 0; i < nprocs_used_; i++)
                std::cerr << ' ' << ((int)host_combuf_->syncstate[i]);
            std::cerr << std::endl;

            // Get the `PROGRAM COUNTER` register (instruction pointer)
            // to see what code is currently being executed
            uint32_t pc[NPROCS];
            for (int i = 0; i < nprocs_used_; i++) {
                e_read(&dev_, (i / cols_), (i % cols_), E_REG_PC, &pc[i],
                       sizeof(uint32_t));
            }

            std::cerr << "Bulk DEBUG: current instruction for every core:";
            for (int i = 0; i < nprocs_used_; i++) {
                if ((i % 4) == 0)
                    std::cerr << "\n\t";
                std::cerr << ' ' << ((void*)pc[i]);
                //Symbol* sym = _get_symbol_by_addr((void*)pc[i]);
                //if (sym)
                //    printf(" %s+%p", sym->name, (void*)(pc[i] - sym->value));
                //else
                //    printf(" %p", (void*)pc[i]);
            }
            std::cerr << '\n';
        }
        ++iter;
#endif
    }

    // Note that this point is not reached when a core calls abort()

    // Flush extmem stream data back to host
    for (auto& s : streams)
        s.flush_stream();

    env_initialized_ = 4;
}

void provider::initialize_() {
    init_application_path_();

    // Initialize the Epiphany system
    if (e_init(NULL) != E_OK) {
        std::cerr << "ERROR: Could not initialize HAL data structures.\n";
        return;
    }

    // Reset the Epiphany system
    if (e_reset_system() != E_OK) {
        std::cerr << "ERROR: Could not reset the Epiphany system.\n";
        return;
    }

    // Get information on the platform
    if (e_get_platform_info(&platform_) != E_OK) {
        std::cerr << "ERROR: Could not obtain platform information.\n";
        return;
    }

    // Obtain the number of processors from the platform information
    rows_ = platform_.rows;
    cols_ = platform_.cols;
    nprocs_available_ = rows_ * cols_;

    env_initialized_ = 1;

    // Open the workgroup
    if (e_open(&dev_, 0, 0, rows_, cols_) != E_OK) {
        std::cerr << "ERROR: Could not open workgroup.\n";
        return;
    }

    // e_alloc will mmap combuf and dynmem
    // The offset in external memory is equal to NEWLIB_SIZE
    if (e_alloc(&emem_, NEWLIB_SIZE, COMBUF_SIZE + DYNMEM_SIZE) != E_OK) {
        std::cerr << "ERROR: e_alloc failed.\n";
        return;
    }
    host_combuf_ = (combuf*)emem_.base;
    malloc_base_ = (void*)(uint32_t(emem_.base) + COMBUF_SIZE);

    ext_malloc_init_();

    env_initialized_ = 2;
}

void provider::finalize_() {
    for (auto& s : streams)
        ext_free_(s.buffer);
    streams.clear();

    if (env_initialized_ >= 2)
        e_free(&emem_);

    if (env_initialized_ >= 1) {
        if (E_OK != e_finalize()) {
            std::cerr << "ERROR: Could not finalize the Epiphany connection."
                      << std::endl;
        }
    }

    env_initialized_ = 0;
}

void provider::set_core_syncstate_(int pid, SYNCSTATE state) {
    // First write it to extmem
    host_combuf_->syncstate[pid] = SYNCSTATE::CONTINUE;

    // Then write it to the core itself
    off_t dst = (off_t)host_combuf_->syncstate_ptr;
    if (e_write(&dev_, (pid / cols_), (pid % cols_), dst, &state,
                sizeof(int8_t)) != sizeof(int8_t)) {
        std::cerr << "ERROR: unable to write syncstate to core memory."
                  << std::endl;
    }
}

// Get the directory that the application is running in
// and store it in e_directory_ including the trailing slash
void provider::init_application_path_() {
    e_directory_.clear();
    char path[1024];
    ssize_t len = readlink("/proc/self/exe", path, 1024);
    if (len > 0 && len < 1024) {
        path[len] = 0;
        char* slash = strrchr(path, '/');
        if (slash) {
            *(slash + 1) = 0;
            e_directory_ = path;
        }
    }
    if (e_directory_.empty()) {
        std::cerr << "ERROR: Could not find process directory.\n";
        e_directory_ = "./";
    }
    return;
}

void provider::update_remote_timer_() {
    clock_gettime(CLOCK_MONOTONIC, &ts_end_);

    float time_elapsed = (ts_end_.tv_sec - ts_start_.tv_sec +
                          (ts_end_.tv_nsec - ts_start_.tv_nsec) * 1.0e-9);

    host_combuf_->remotetimer = time_elapsed;
}

void provider::microsleep_(int microseconds) {
    struct timespec request, remain;
    request.tv_sec = (int)(microseconds / 1000000);
    request.tv_nsec = (microseconds - 1000000 * request.tv_sec) * 1000;
    if (clock_nanosleep(CLOCK_MONOTONIC, 0, &request, &remain) != 0)
        std::cerr << "ERROR: clock_nanosleep was interrupted." << std::endl;
}

#include "malloc_implementation.cpp"

void provider::ext_malloc_init_() {
    uint32_t size = DYNMEM_SIZE;
    // possibly round up if combuf is not multiple of chunk size
    void* new_base = (void*)chunk_roundup((uint32_t)malloc_base_);
    if (new_base != malloc_base_) {
        size = size - (uint32_t(new_base) - uint32_t(malloc_base_));
        malloc_base_ = new_base;
        std::cerr << "ERROR: External malloc base is not aligned." << std::endl;
    }
    _init_malloc_state(malloc_base_, size);
}

void* provider::ext_malloc_(uint32_t size) {
    return _malloc(malloc_base_, size);
}

void provider::ext_free_(void* ptr) {
    return _free(malloc_base_, ptr);
}

} // namespace epiphany
} // namespace bulk
