#pragma once

extern "C" {
#define MCBSP_COMPATIBILITY_MODE
#include <mcbsp.h>
}

namespace bulk {
namespace bsp {

class world_provider {
  public:
    world_provider() { tag_size_ = 0; }

    int active_processors() const { return bsp_nprocs(); }
    int processor_id() const { return bsp_pid(); }

    void sync() const { bsp_sync(); }

    void internal_put_(int processor, void* value, void* variable, size_t size,
                       int offset, int count) {
        bsp_put(processor, value, variable, offset * size, count * size);
    }

    int register_location_(void* location, size_t size) {
        bsp_push_reg(location, size);
        sync();
        return 0; // bsp provider does not use var identifiers yet
    }

    void unregister_location_(void* location) {
        bsp_pop_reg(location);
    }

    void internal_get_(int processor, void* variable, void* target, size_t size,
                       int offset, int count) {
        bsp_get(processor, variable, offset * size, target, count * size);
    }

    virtual void internal_send_(int processor, void* tag, void* content,
                                size_t tag_size, size_t content_size) {
        if (tag_size_ != tag_size) {
            int tag_size_copy = tag_size;
            bsp_set_tagsize(&tag_size_copy);
            sync();

            tag_size_ = tag_size;
        }

        bsp_send(processor, tag, content, content_size);
    }

  private:
    size_t tag_size_ = 0;
};

} // namespace bsp
}
