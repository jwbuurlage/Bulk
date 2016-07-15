#include <utility.hpp>
#include <backend.hpp>

// This variable indicates end of global vars (end of .bss section)
// So 'end' until 'stack' can be used by malloc
extern int end;

namespace bulk {
namespace epiphany {

#include "malloc_implementation.cpp"

// This class is made so that the constructor is called before main
// This way, the malloc system is started independently from the hub
class malloc_starter_ {
  public:
    malloc_starter_() {
        ptr = (void*)chunk_roundup((uint32_t)(&end + 8));
        uint32_t size = 0x8000 - (uint32_t)ptr;
        _init_malloc_state(ptr, size);
    }
    ~malloc_starter_() {}
    void* ptr;
};

malloc_starter_
    malloc_instance_; // global so that it is initialized before main

void* EXT_MEM_TEXT ext_malloc(unsigned int nbytes) {
    void* ret = 0;
    world.implementation().mutex_lock_(MUTEX_EXTMALLOC);
    ret = _malloc((void*)E_DYNMEM_ADDR, nbytes);
    world.implementation().mutex_unlock_(MUTEX_EXTMALLOC);
    return ret;
}

const char err_allocation[] EXT_MEM_RO =
    "BULK ERROR: allocation of %d bytes of local memory overwrites the stack";

void* EXT_MEM_TEXT malloc(unsigned int nbytes) {
    void* ret = 0;
    ret = _malloc(malloc_instance_.ptr, nbytes);

    // Must check for zero because using nbytes > ~0x8000
    // will give ret = 0, and then it will trigger the next
    // if statement and try to free the null pointer
    if (ret == 0)
        return 0;

    // Check if it does not overwrite the current stack position
    // Plus 128 bytes of margin
    if ((uint32_t)ret + nbytes + 128 > (uint32_t)&ret) // <-- only epiphany
    {
        _free(malloc_instance_.ptr, ret);
        bulk::epiphany::print(err_allocation, nbytes);
        return 0;
    }
    return ret;
}

void EXT_MEM_TEXT free(void* ptr) {
    if (((unsigned)ptr) & 0xfff00000) {
        world.implementation().mutex_lock_(MUTEX_EXTMALLOC);
        _free((void*)E_DYNMEM_ADDR, ptr);
        world.implementation().mutex_unlock_(MUTEX_EXTMALLOC);
    } else {
        _free(malloc_instance_.ptr, ptr);
    }
}
}
}
