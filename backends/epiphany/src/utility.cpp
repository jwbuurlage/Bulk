#include "world_state.hpp"
#include <cstdio>
#include <cstdarg>

namespace bulk {
namespace epiphany {

void memcpy(void* dest, const void* source, unsigned int nbytes) {
    unsigned bits = (unsigned)dest | (unsigned)source;
    if ((bits & 0x7) == 0) {
        // 8-byte aligned
        long long* dst = (long long*)dest;
        const long long* src = (const long long*)source;
        int count = nbytes >> 3;
        nbytes &= 0x7;
        while (count--)
            *dst++ = *src++;
        dest = (void*)dst;
        source = (void*)src;
    } else if ((bits & 0x3) == 0) {
        // 4-byte aligned
        uint32_t* dst = (uint32_t*)dest;
        const uint32_t* src = (const uint32_t*)source;
        int count = nbytes >> 2;
        nbytes &= 0x3;
        while (count--)
            *dst++ = *src++;
        dest = (void*)dst;
        source = (void*)src;
    }

    // do remaining bytes 1-byte aligned
    char* dst_b = (char*)dest;
    const char* src_b = (const char*)source;
    while (nbytes--)
        *dst_b++ = *src_b++;
}

void EXT_MEM_TEXT print(const char* format, ...) {
    // Lock mutex
    state.mutex_lock_(MUTEX_PRINT);

    // Write the message to a buffer
    char buf[128];
    va_list args;
    va_start(args, format);
    vsnprintf(&buf[0], sizeof(buf), format, args);
    va_end(args);

    // Write the message
    bulk::epiphany::memcpy(&combuf_->msgbuf[0], buf, sizeof(combuf_->msgbuf));

    // Wait for message to be written
    state.write_syncstate_(SYNCSTATE::MESSAGE);
    while (state.syncstate_ != SYNCSTATE::CONTINUE) {
    };
    state.write_syncstate_(SYNCSTATE::RUN);

    // Unlock mutex
    state.mutex_unlock_(MUTEX_PRINT);
}

float host_time() {
    return combuf_->remotetimer;
}

}
}
