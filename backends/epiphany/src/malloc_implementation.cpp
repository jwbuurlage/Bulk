#pragma once
#include <cstdint>

// This file is used on both the host and epiphany side
// On the epiphany side the functions are stored in extmem
#ifdef __epiphany__
#define MALLOC_FUNCTION_PREFIX EXT_MEM_TEXT
#else
#define MALLOC_FUNCTION_PREFIX
#endif

//
// Layout of memory:
//      First one uint32_t with total size
//      Then one uint32_t of padding
//      Then a linked list of `memory_object`.
//      Last `memory_object` has chunk_size set to zero.
//      To see if there is space after the last memory object,
//      use the total size at the beginning. We do NOT want to
//      set a final memory_object at the end to finish the linked
//      list because this does not work wil the Epiphany stack.

// Allocated chunks are 8-byte (64-bit) aligned
// so that they can be used by for example the DMA engine
constexpr uint32_t CHUNK_SIZE = 8;

// Every chunk starts with the follwoing structs
// which must be exactly 8 bytes, after which the
// actual allocated memory starts
// This means every malloc will use up at least 16 bytes
typedef struct {
    // Both sizes are *including* the `memory_object`.
    // Since sizes are multiples of CHUNK_SIZE we can safely use
    // the lowest bit of chunk_size as a flag.
    uint32_t chunk_size; // bit 0 enabled means 'in use' or allocated.
    uint32_t prev_size;
} memory_object;

typedef struct {
    uint32_t total_size;
    uint32_t padding;
    memory_object first_chunk;
} malloc_region;

inline bool mem_in_use(memory_object* obj) {
    return (obj->chunk_size & 1) != 0;
}

// Round up to the next multiple of CHUNK_SIZE only if not a multiple yet
inline uint32_t chunk_roundup(uint32_t a) {
    // Compiler optimizes this function to (((a+7)>>3)<<3)
    // I also tested ((a+7) & ~7)
    // but this is 4 extra bytes of assembly and
    // takes exactly 1 extra clockcycle
    return ((a + CHUNK_SIZE - 1) / CHUNK_SIZE) * CHUNK_SIZE;
}

// ebsp_ext_malloc wraps this in a mutex
void* MALLOC_FUNCTION_PREFIX _malloc(void* base, uint32_t nbytes) {
    uint32_t required_size = chunk_roundup(nbytes + sizeof(memory_object));

    malloc_region* region = (malloc_region*)base;

    uint32_t result = 0;
    memory_object* cur = &(region->first_chunk);
    for (;;) {
        // chunk_size includes the size of memory_object
        uint32_t chunk_size = (cur->chunk_size & ~0x1);
        if (!mem_in_use(cur)) {
            if (chunk_size == 0) {
                // This is the last element in the list
                // Check if it will fit:
                //  cur + 0x00          : memory_object already here
                //  cur + 0x08          : newly allocated region
                //  cur + 0x08 + nbytes : new memory_object
                //  cur + 0x10 + nbytes : end
                // Note that `required_size` is 0x08 + nbytes
                // and nbytes should always be rounded up

                uint32_t new_end =
                    uint32_t(cur) + required_size + sizeof(memory_object);
                uint32_t real_end = uint32_t(base) + region->total_size;
                if (new_end <= real_end) {
                    memory_object* next =
                        (memory_object*)(uint32_t(cur) + required_size);
                    next->chunk_size = 0;
                    next->prev_size = required_size;

                    cur->chunk_size = required_size | 1;
                    result = uint32_t(cur);
                }
                break;
            } else if (chunk_size >= required_size) {
                // Not the last element, and it fits.
                // Add it to the linked list

                // If there is nbytes + 8 or less space
                // then we can not create a new memory_object
                // after it because it would have zero size.
                // In that case, just enlarge this memory object
                if (chunk_size <= required_size + sizeof(memory_object)) {
                    // This possibly enlarges required size by 8
                    cur->chunk_size |= 1;
                    result = uint32_t(cur);
                } else {
                    memory_object* next =
                        (memory_object*)(uint32_t(cur) + required_size);
                    next->chunk_size = chunk_size - required_size;
                    next->prev_size = required_size;
                    cur->chunk_size = required_size | 1;
                    result = uint32_t(cur);
                }
                break;
            }
        }
        cur = (memory_object*)(uint32_t(cur) + chunk_size);
    }

    if (result)
        return (void*)(result + sizeof(memory_object));
    return 0;
}

void MALLOC_FUNCTION_PREFIX _merge_memory_objects(memory_object* obj) {
    if (mem_in_use(obj))
        return;

    memory_object* next = (memory_object*)(uint32_t(obj) + obj->chunk_size);

    if (mem_in_use(next))
        return;

    // we can merge this memory object with the next one
    // effectively deleting the next memory object from the linked list
    if (next->chunk_size == 0) {
        // next was the last block, so that makes obj the new last block
        obj->chunk_size = 0;
    } else {
        // delete the next memory object from the linked list
        obj->chunk_size += next->chunk_size; // set cur->forward
        // go to new next and set its prev size
        next = (memory_object*)(uint32_t(obj) + obj->chunk_size);
        next->prev_size = obj->chunk_size; // set next->backward
    }
}

void MALLOC_FUNCTION_PREFIX _free(void* base, void* ptr) {
    memory_object* obj;
    obj = (memory_object*)(uint32_t(ptr) - sizeof(memory_object));

    // First set it to 'free'
    obj->chunk_size &= ~0x1;

    // If there are free regions before or after this one, link them together
    // so that this does not have to be done in malloc.
    // Note that if obj it the very first chunk, then prev_size is zero
    memory_object* prev = (memory_object*)(uint32_t(obj) - obj->prev_size);

    _merge_memory_objects(obj);
    _merge_memory_objects(prev);
    return;
}

// Initializes the malloc table
void MALLOC_FUNCTION_PREFIX _init_malloc_state(void* base, uint32_t size) {
    malloc_region* region = (malloc_region*)base;
    region->total_size = size;
    region->first_chunk.chunk_size = 0;
    region->first_chunk.prev_size = 0;
}

// For debugging: get amount of memory in use
void MALLOC_FUNCTION_PREFIX _malloc_mem_info(void* base, uint32_t* total_size,
                                             uint32_t* used_size,
                                             uint32_t* used_count) {
    malloc_region* region = (malloc_region*)base;

    uint32_t used = 0;
    uint32_t count = 0;

    memory_object* cur = &(region->first_chunk);
    for (;;) {
        // chunk_size includes the size of memory_object
        uint32_t chunk_size = (cur->chunk_size & ~0x1);
        if (chunk_size == 0)
            break;
        if (mem_in_use(cur)) {
            used += chunk_size;
            count++;
        }
        cur = (memory_object*)(uint32_t(cur) + chunk_size);
    }

    *total_size = region->total_size;
    *used_size = used;
    *used_count = count;

    return;
}
