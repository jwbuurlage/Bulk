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
     * Get the local rank
     *
     * \returns an integer containing the rank of the local processor
     */
    virtual int rank() const = 0;

    [[deprecated("`world::processor_id` was renamed to `world::rank`")]] int
    processor_id() const {
        return this->rank();
    }

    /**
     * Get the rank of the next logical processor
     *
     * \returns an integer containing the rank of the next processor
     */
    int next_rank() const {
        auto next = rank() + 1;
        if (next >= active_processors())
            next -= active_processors();
        return next;
    }

    [[deprecated("`world::next_processor` was renamed to "
                 "`world::next_rank`")]] int
    next_processor() const {
        return this->next_rank();
    }

    /**
     * Get the rank of the previous logical processor
     *
     * \returns an integer containing the rank of the previous processor
     */
    int prev_rank() const {
        auto prev = rank() + active_processors() - 1;
        if (prev >= active_processors())
            prev -= active_processors();
        return prev;
    }

    [[deprecated("`world::prev_processor` was renamed to "
                 "`world::prev_rank`")]] int
    prev_processor() const {
        return prev_rank();
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
     * The logs are sorted by processor rank before being printed at the next
     * call to `sync`.
     *
     * At the next call to `sync`, the logs of all processors are sent
     * to the `environment` and by default printed to standard output.
     * Instead one can use `environment::set_log_callback` to intercept
     * these log messages.
     */
    void log(const char* format) {
        size_t size = snprintf(nullptr, 0, "%s", format);
        char* buffer = new char[size + 1];
        snprintf(buffer, size + 1, "%s", format);
        log_(std::string(buffer));
        delete[] buffer;
    }

    template <typename... Ts, std::enable_if_t<sizeof...(Ts) != 0, int> = 0>
    void log(const char* format, const Ts&... ts) {
        size_t size = snprintf(nullptr, 0, format, ts...);
        char* buffer = new char[size + 1];
        snprintf(buffer, size + 1, format, ts...);
        log_(std::string(buffer));
        delete[] buffer;
    }

    /** Log only with the first logical processor */
    template <typename... Ts>
    void log_once(const char* format, const Ts&... ts) {
        if (rank() == 0) {
            log(format, ts...);
        }
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

    /**
     * Split into a number of 'subworlds'.
     *
     * Every processor in the same part after splitting get assigned consecutive
     * 0-based ranks in the subworld in ascending order from their rank in the
     * ambient world.
     *
     * The subworld is returned as a unique pointer, to ensure proper clean up
     * when it is no longer needed.
     */
    virtual std::unique_ptr<bulk::world> split(int part) = 0;

  protected:
    // Internal functions
    template <typename T>
    friend class var;

    template <typename T>
    friend class future;

    template <typename T>
    friend class array;

    template <typename T>
    friend class coarray;

    template <typename... Ts>
    friend class queue;

    // Returns the id of the registered location
    virtual int register_variable_(class var_base* location) = 0;
    virtual void unregister_variable_(int id) = 0;

    // Futures
    virtual int register_future_(class future_base* future) = 0;
    virtual void unregister_future_(class future_base* future) = 0;

    virtual char* put_buffer_(int processor, int var_id, size_t size) = 0;
    // size is per element
    virtual void
    put_(int processor, const void* values, size_t size, int var_id, size_t offset, size_t count) = 0;

    virtual void get_buffer_(int processor, int var_id, class future_base* future) = 0;
    virtual void
    get_(int processor, int var_id, size_t size, void* target, size_t offset, size_t count) = 0;

    virtual int register_queue_(class queue_base* q) = 0;
    virtual void unregister_queue_(int id) = 0;

    // data consists of a complete message. size is total size.
    virtual char* send_buffer_(int target, int queue_id, size_t buffer_size) = 0;

    virtual void log_(std::string message) = 0;
};

} // namespace bulk
