#pragma once
#include <iostream>
#include <string>
#include <cstdint>
#include <cstring>
#include <functional>

extern "C" {
#include <e-hal.h>
#include <e-loader.h>
}

#include "combuf.hpp"

namespace bulk {
namespace epiphany {

class provider {
  public:
    //using world_provider_type = world_provider;

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

    // Initialize the external memory malloc system
    void malloc_init_();
};

} // namespace epiphany
} // namespace bulk
