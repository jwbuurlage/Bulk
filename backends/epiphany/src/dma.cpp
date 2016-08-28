#include "dma.hpp"

extern "C" {
#include <e_ic.h>
#include <e_regs.h>
}

namespace bulk {
namespace epiphany {

// Globals. Could be part of world, but this is also fine
dma_task* last_task[2];
dma_task* cur_task[2];
unsigned* dmaconfig[2];

void __attribute__((interrupt)) _dma_interrupt0();
void __attribute__((interrupt)) _dma_interrupt1();

constexpr unsigned B_OPCODE = 0x000000e8; // branch

void init_dma_handlers() {
    // Taken from `e_irq_attach` and `e_irq_mask`

    // Disable global interrupts
    __asm__("gid");

    // Attach handlers to interrupt-vector-table
    unsigned *ivt;
    ivt = (unsigned*)(E_DMA0_INT << 2);
    *ivt = (((unsigned)&_dma_interrupt0 - (unsigned)ivt) >> 1) << 8;
    *ivt = *ivt | B_OPCODE;
    ivt = (unsigned*)(E_DMA1_INT << 2);
    *ivt = (((unsigned)&_dma_interrupt1 - (unsigned)ivt) >> 1) << 8;
    *ivt = *ivt | B_OPCODE;

    // Clear IMASK bits for DMA1 and DMA2
    unsigned coreid;
    unsigned* imask;
    __asm__("movfs %0, coreid" : "=r"(coreid));
    imask = (unsigned*)((coreid << 20) | E_REG_IMASK);
    *imask &= ~((1 << (E_DMA0_INT - E_SYNC)) | (1 << (E_DMA1_INT - E_SYNC)));

    // Enable global interrupts
    __asm__("gie");

    // `last_task` and `cur_task` are set to zero by the program loader
    dmaconfig[0] = (unsigned*)((coreid << 20) | E_REG_DMA0CONFIG);
    dmaconfig[1] = (unsigned*)((coreid << 20) | E_REG_DMA1CONFIG);
}

// Push this task to the dma engine
// Assumes the task has been set through `set`
void dma_task::push(int engine) {
    // Take the end of the current descriptor chain
    dma_task* last = last_task[engine];

    if (last == NULL) {
        // No current chain, replace it by this one
        last_task[engine] = this;
    } else if (last != this) {
        // We need to disable interrupts because
        // if the interrupt fires between `newconfig = ...`
        // and `last->config = ...` then the interrupt
        // clears the E_DMA_ENABLE bit but it is then set again
        // by this code below, which is not what we want
        __asm__("gid");
        // Attach desc to last
        unsigned newconfig =
            (last->config & 0x0000ffff) | ((unsigned)this << 16);
        last->config = newconfig;
        __asm__("gie");
        last_task[engine] = this;
    }

    // In principle it could happen that at this point, the previous
    // DMA task finished, its interrupt fires which starts the DMA
    // engine for THIS task, which could then ALSO finish.
    // Therefore we have to check for this inside the if statement below

    // Start DMA if not started yet
    if (cur_task[engine] == 0) {
        // Check if this task (being pushed) has not finished yet
        if (config & E_DMA_ENABLE) {
            // Start the DMA engine using the kickstart bit
            cur_task[engine] = this;
            unsigned kickstart = ((unsigned)this << 16) | E_DMA_STARTUP;
            *dmaconfig[engine] = kickstart;
        }
    }
}

void _dma_interrupt_(int engine) {
    // If DMA is in chaining mode, an interrupt will be fired after a chain
    // element is completed. At this point in the interrupt, the DMA will
    // already be busy doing the next element of the chain or even the one
    // after that if it fired two interrupts really quickly after each other.
    //
    // The DMA should not be used in chaining mode, or the whole method
    // does not work. It seems like the codeblock below is a solution
    // but it fails when there are many tiny dma transfers in the chain
    // and the DMA fires many interrupts that will be lost in obliviion
    // because this function is still running.
    // We can not lose any interrupts, because we need to advance the
    // `cur_dma_desc` pointer at each interrupt, even for chains
    //
    // unsigned status = *dmastatus[engine];
    // if (status & 0xf)
    //     return;

    // Grab the current task
    dma_task* cur = cur_task[engine];
    // Mark as finished
    cur->config &= ~(E_DMA_ENABLE);

    // Go to the 'next' task
    dma_task* next = (dma_task*)(cur->config >> 16);
    cur_task[engine] = next;

    if (next) {
        // Start the DMA engine using the kickstart bit
        unsigned kickstart = ((unsigned)next << 16) | E_DMA_STARTUP;
        *dmaconfig[engine] = kickstart;
    }
}

void __attribute__((interrupt)) _dma_interrupt0() { return _dma_interrupt_(0); }
void __attribute__((interrupt)) _dma_interrupt1() { return _dma_interrupt_(1); }

} // namespace epiphany
} // namespace bulk
