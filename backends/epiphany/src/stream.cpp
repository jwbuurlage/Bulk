#include "stream.hpp"
#include "utility.hpp"
#include "world_state.hpp"

namespace bulk {
namespace epiphany {

const char err_stream_id_[] EXT_MEM_RO =
    "BULK ERROR: stream with id %d does not exist";

const char err_stream_in_use_[] EXT_MEM_RO =
    "BULK ERROR: stream with id %d was in use by another processor";

void stream::open(int id) {
    if (id < 0 || id > combuf_->nstreams) {
        print(err_stream_id_, id);
        return;
    }

    stream_descriptor* desc = &(combuf_->streams[id]);
    int mypid = state.processor_id();

    state.mutex_lock_(MUTEX_STREAM);
    if (desc->pid == -1) {
        desc->pid = mypid;
        mypid = -1;
    }
    state.mutex_unlock_(MUTEX_STREAM);

    if (mypid != -1) {
        print(err_stream_in_use_, id);
        return;
    }

    // We succesfully opened the stream

    // Copy descriptor data
    buffer = desc->buffer;
    capacity = desc->capacity;
    offset = desc->offset;
    size = desc->size;

    // Set local values
    stream_id = id;
    cursor = buffer;
}

} // namespace epiphany
} // namespace bulk
