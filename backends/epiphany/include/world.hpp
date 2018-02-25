#pragma once
#include <bulk/future.hpp>
#include <bulk/world.hpp>
#include <bulk/variable.hpp>
#include "epiphany_internals.hpp"
#include "utility.hpp"
#include "world_state.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>

namespace bulk {
namespace epiphany {

class world : public bulk::world {
  public:
    world() {}
    ~world() {};

    int active_processors() const override { return state.active_processors(); }
    int rank() const override { return state.processor_id(); }

    void sync() override {
        state.barrier();

        // Gets to this processor
        for (auto& t : coarray_get_tasks_) {
            bulk::epiphany::memcpy(t.dst, t.src, t.size);
        }
        coarray_get_tasks_.clear();

        // TODO: optimize the redundant buffer away
        for (auto& t : var_get_tasks_) {
            auto size = t.var->serialized_size();
            char* buffer = new char[size];
            t.var->serialize(buffer);
            t.future->deserialize_get(size, buffer);
            delete[] buffer;
        }
        var_get_tasks_.clear();

        state.barrier();
       
        // Puts from this processor
        for (auto& t : coarray_put_tasks_) {
            memcpy(t.dst, t.src, t.size);
        }
        coarray_put_tasks_.clear();

        // This does var put tasks and queues
        state.sync();

        state.barrier();
    }

    /// Barrier that not resolve outstanding communication like sync.
    void barrier() override { state.barrier(); }

    // overrule the log in the base class
    // since the printf-family is broken on epiphany
    template <typename... Ts>
    void log(const char* format, const Ts&... ts) {
        print(format, ts...);
    }

    void abort() override { state.abort(); }

    int register_variable_(class var_base* varbase) override {
        auto locsize = varbase->location_and_size();
        return (int)state.register_location_(varbase, locsize.first,
                                             locsize.second);
    }

    void unregister_variable_(int id) override {
        state.unregister_location_(id);
    }

    int register_future_(class future_base* future) override { return 0; }

    void unregister_future_(class future_base* future) override { return; }

    char* put_buffer_(int processor, int var_id, size_t size) override {
        return state.put_buffer_(processor, var_id, size);
    }

    // size is per element
    void put_(int processor, const void* values, size_t size,
                      int var_id, size_t offset, size_t count) override {
        auto& v = state.get_var_(processor, var_id);
        if (size * (offset + count) > v.capacity) {
            log("BULK ERROR: array put out of bounds");
            return;
        }
        bulk::epiphany::memcpy(v.receiveBuffer + size * offset, values, size * count);
        // TODO
        coarray_put_tasks_.push_back({(char*)v.buffer + size * offset,
                              v.receiveBuffer + size * offset, size * count});
        return;
    }

    void get_buffer_(int processor, int var_id, class future_base* future) override {
        var_get_tasks_.push_back({future, state.get_var_(processor, var_id).base});
        return;
    }
    void get_(int processor, int var_id, size_t size, void* target,
                      size_t offset, size_t count) override {
        coarray_get_tasks_.push_back(
            {target, (char*)state.get_var_(processor, var_id).buffer + size * offset,
             size * count});
        return;
    }

    int register_queue_(class queue_base* q) override {
        return state.register_queue_(q);
    }

    void unregister_queue_(int id) override { state.unregister_queue_(id); }

    // data consists of a complete message. size is total size.
    char* send_buffer_(int processor, int queue_id, size_t size) override {
        char* buffer = new char[size];
        // Lock mutex only to append the pointer to vector
        auto& q = state.get_queue_(queue_id, processor);
        state.mutex_lock_(&q.mutex);
        // TODO: vectors should only be accessed from the
        // core that created it because new[]/delete[] should
        // always be called by the same core !
        q.receiveBuffers.push_back(std::make_pair(buffer, size));
        state.mutex_unlock_(&q.mutex);
        return buffer;
    }

    // not used in epiphany backend
    // the main world::log function is overloaded
    void log_(std::string message) override {
        return;
    }

    // Explicitly send finish signal to host
    void finalize() {
        state.finalize();
    }

  private:
    // Task for data that needs no serialization
    struct copy_task {
        void* dst;
        void* src;
        size_t size;
    };
    // Task for data that needs serialization
    struct get_task {
        class future_base* future; // dst
        class var_base* var;       // src
    };

    std::vector<get_task> var_get_tasks_;
    std::vector<copy_task> coarray_get_tasks_;
    std::vector<copy_task> coarray_put_tasks_;
};

} // namespace epiphany
} // namespace bulk

