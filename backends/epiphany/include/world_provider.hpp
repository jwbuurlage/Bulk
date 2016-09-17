#pragma once
#include "epiphany_internals.hpp"
#include "utility.hpp"
#include "world_state.hpp"
#include <cstddef>
#include <cstdint>

namespace bulk {
namespace epiphany {

class world_provider {
  public:
    world_provider() {}
    ~world_provider() {};

    int active_processors() const { return state.active_processors(); }
    int processor_id() const { return state.processor_id(); }

    void sync() { state.sync(); }

    /// Barrier that not resolve outstanding communication like sync.
    void barrier() { state.barrier(); }

    template <typename... Ts>
    void log(const char* format, const Ts&... ts) {
        print(format, ts...);
    }

    void abort() { state.abort(); }

    var_id_t register_location_(void* location, size_t size) {
        return state.register_location_(location, size);
    }

    void move_location_(int id, void* newlocation) {
        state.move_location_(id, newlocation);
    }

    void unregister_location_(void* location) {
        state.unregister_location_(location);
    }

    // This gets an int from `world` so do not use `var_id_t` here.
    void unregister_location_(int id) { state.unregister_location_(id); }

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
};

} // namespace epiphany
} // namespace bulk

