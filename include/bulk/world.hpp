#pragma once

#include <cstddef>
#include <string>

/**
 * \file world.hpp
 *
 * This objects represents the world of a processor and its place within it.
 */

namespace bulk {

/**
 * This objects represents the world of a processor and its place within it.
 */
class world {
  public:
    world(){};
    virtual ~world(){};

    // Disallow copies
    world(world& other) = delete;
    void operator=(world& other) = delete;

    /**
     * Get the total number of active processors in a spmd section
     *
     * \returns the number of active processors
     */
    virtual int active_processors() const = 0;

    /**
     * Get the local processor id
     *
     * \returns an integer containing the id of the local processor
     */
    virtual int processor_id() const = 0;

    /**
     * Get the id of the next logical processor
     *
     * \returns an integer containing the id of the next processor
     */
    int next_processor() const {
        auto next = processor_id() + 1;
        if (next >= active_processors())
            next -= active_processors();
        return next;
    }

    /**
     * Get the id of the previous logical processor
     *
     * \returns an integer containing the id of the previous processor
     */
    int prev_processor() const {
        auto prev = processor_id() + active_processors() - 1;
        if (prev >= active_processors())
            prev -= active_processors();
        return prev;
    }

    /**
     * Perform a global barrier synchronization of the active processors
     * and resolves any outstanding communication. Messages previously
     * received in queues are cleared for the next superstep.
     * The function must be called by all processors.
     * When some processors call `sync` while others call `barrier`
     * at the same time, behaviour is undefined.
     */
    virtual void sync() = 0;

    /**
     * Perform a global barrier synchronization of the active processors
     * without resolving outstanding communication. Queues are not cleared.
     * The function must be called by all processors.
     * When some processors call `sync` while others call `barrier`
     * at the same time, behaviour is undefined.
     */
    virtual void barrier() = 0;

    /**
     * Print output for debugging, shown at the next synchronization.
     *
     * The syntax is printf style.
     *
     * The logs are sorted by processor id before being printed at the next
     * call to `sync`.
     *
     * At the next call to `sync`, the logs of all processors are sent
     * to the `environment` and by default printed to standard output.
     * Instead one can use `environment::set_log_callback` to intercept
     * these log messages.
     */
    template <typename... Ts>
    void log(const char* format, const Ts&... ts) {
        size_t size = snprintf(0, 0, format, ts...);
        char* buffer = new char[size + 1];
        snprintf(buffer, size + 1, format, ts...);
        log_(std::string(buffer));
        delete[] buffer;
    }

    /**
     * Terminate the spmd program on all processors.
     *
     * This is not the normal way to exit a bulk program
     * and indicates an error has occurred.
     * If any processor calls abort, all processors will stop
     * and `bulk::environment::spawn` will throw an exception.
     */
    virtual void abort() = 0;

  protected:
    //
    // Internal functions
    //
    template <typename T>
    friend class var;

    template <typename T>
    friend class array;

    template <typename T>
    friend class coarray;

    template <typename... Ts>
    friend class queue;

    // Returns the id of the registered location
    virtual int register_location_(void* location, size_t size) = 0;
    virtual void unregister_location_(int id) = 0;

    virtual void put_(int processor, const void* value, size_t size,
                      int var_id) = 0;
    // size is per element
    virtual void put_(int processor, const void* values, size_t size,
                      int var_id, size_t offset, size_t count) = 0;

    virtual void get_(int processor, int var_id, size_t size, void* target) = 0;
    // size is per element
    virtual void get_(int processor, int var_id, size_t size, void* target,
                      size_t offset, size_t count) = 0;

    virtual int register_queue_(class queue_base* q) = 0;
    virtual void unregister_queue_(int id) = 0;

    // data consists of a complete message. size is total size.
    virtual char* send_buffer_(int target, int queue_id, size_t buffer_size) = 0;

    virtual void log_(std::string message) = 0;
};

} // namespace bulk
