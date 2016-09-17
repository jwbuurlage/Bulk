#pragma once
#include "epiphany_internals.hpp"
#include "dma.hpp"
#include "world_state.hpp"

namespace bulk {
namespace epiphany {

class stream {
  public:
    stream() { stream_id = -1; }
    stream(int id) {
        stream_id = -1;
        open(id);
    }
    ~stream() { close(); }

    // See `dma_task` for why it can not be copied or moved
    stream(stream&) = delete;
    void operator=(stream&) = delete;
    stream(stream&&) = delete;
    void operator=(stream&&) = delete;

    // Open a stream. If a different stream was open, close it
    // If the same stream was open, dont do anything.
    void open(int id);

    void close() {
        if (stream_id == -1)
            return;
        wait();
        // Should not have to lock mutex for this atomic write
        combuf_->streams[stream_id].pid = -1;
        stream_id = -1;
    }

    bool is_valid() const { return stream_id != -1; }

    operator bool() const { return is_valid(); }

    // Wait for any pending asynchroneous operations to complete
    void wait() { dma.wait(); }

    // Seek with relative offset
    void seek_rel(int delta_bytes) {
        if (delta_bytes <= 0) {
            cursor = (void*)((unsigned)cursor + delta_bytes);
            if (cursor < buffer) {
                // TODO: host request
                // First write/flush old data then request new data ?
                state.write_syncstate_(SYNCSTATE::STREAMREQ);
                while (state.syncstate_ != SYNCSTATE::CONTINUE) {
                };
                state.write_syncstate_(SYNCSTATE::RUN);

                cursor = buffer;
            }
        } else {
            size_t remaining =
                (unsigned)buffer + filled_size - (unsigned)cursor;
            if ((unsigned)delta_bytes > remaining) {
                // TODO: host request
                // First write/flush old data then request new data ?
                state.write_syncstate_(SYNCSTATE::STREAMREQ);
                while (state.syncstate_ != SYNCSTATE::CONTINUE) {
                };
                state.write_syncstate_(SYNCSTATE::RUN);

                delta_bytes = remaining;
            }
            cursor = (void*)((unsigned)cursor + delta_bytes);
        }
    }

    // Seek with absolute offset from the start of the stream
    void seek_abs(size_t offset_) {
        // The buffer
        // [buffer, buffer + capacity[
        // corresponds to the stream part
        // [offset, offset + capacity[
        // Now we request the cursor to go to `offset_`
        // First check if that lies within the current loaded stream part
        if ((int)offset_ < offset || offset_ > offset + capacity) {
            // TODO: host seek request
            // First write/flush old data then request new data ?
            state.write_syncstate_(SYNCSTATE::STREAMREQ);
            while (state.syncstate_ != SYNCSTATE::CONTINUE) {
            };
            state.write_syncstate_(SYNCSTATE::RUN);

            cursor = buffer;
        } else {
            // buffer corresponds to offset
            // buffer + offset_ - offset corresponds to offset_
            cursor = (void*)((unsigned)buffer + offset_ - offset);
        }
    }

    /**
     * Write local data up to a stream.
     *
     * @param data The data to be sent up the stream
     * @param data_size The size of the data to be sent, i.e. the size of the
     * token.
     * @param wait_for_completion If true this function blocks untill
     * the data is completely written to the stream.
     * @return Number of bytes written, negative on error.
     *
     * The function *always* waits for any previous transfers to finish.
     *
     * If `wait_for_completion` is nonzero, this function will wait untill
     * the data is transferred. This corresponds to single buffering.
     *
     * Alternativly, double buffering can be used as follows.
     * Set `wait_for_completion` to zero and continue constructing the next
     * token
     * in a different buffer. Usage example:
     * \code{.c}
     * int* buf1 = new int[100];
     * int* buf2 = new int[100];
     * int* curbuf = buf1;
     * int* otherbuf = buf2;
     *
     * stream s(0); // open strema 0
     * while (...) {
     *     // Fill curbuf
     *     for (int i = 0; i < 100; i++)
     *         curbuf[i] = 5;
     *
     *     // Send up
     *     s.write(curbuf, 100 * sizeof(int), false);
     *     // Use other buffer
     *     swap(curbuf, otherbuf);
     * }
     * delete[] buf1;
     * delete[] buf2;
     * \endcode
     *
     * @remarks Memory is transferred using the `DMA1` engine.
     */
    int write(void* data, size_t size, bool wait_for_completion) {
        size_t remaining = (unsigned)buffer + capacity - (unsigned)cursor;
        if (size > remaining) {
            // TODO: a blocking write to host
            state.write_syncstate_(SYNCSTATE::STREAMWRITE);
            while (state.syncstate_ != SYNCSTATE::CONTINUE) {
            };
            state.write_syncstate_(SYNCSTATE::RUN);
            return -1;
        }
        // Wait for any previous transfer to finish (either down or up)
        wait();
        // Round size up to a multiple of 8
        // If this is not done, integer access to the headers will crash
        // TODO: we dont do this anymore since there are no interleaved
        // headers in the stream. However the docs should very clearly
        // state that if the size is not a multiple of 8 then its very slow
        // size = ((size + 8 - 1) / 8) * 8;

        // Write the data (async)
        dma.push(cursor, data, size, 1);
        cursor = (void*)((unsigned)cursor + size);

        // TODO:
        // Send async host write request.
        // Reason: if you dont do it right now and the kernel does `seek`
        // to another part of the stream, the written data is lost

        if (wait_for_completion)
            wait();
        return size;
    }

    /**
     * Read data from a stream to a local buffer.
     *
     * @param dst_buf Buffer that receives the data. Must hold at least size.
     * @param size Size of the buffer.
     * @param wait_for_completion If true this function blocks untill
     * the data is completely read from the stream.
     * @return Number of bytes read. Zero at end of stream, negative on error.
     *
     * The function *always* waits for any previous transfers to finish.
     *
     * @remarks Memory is transferred using the `DMA1` engine.
     */
    int read(void* dst_buf, size_t size, bool wait_for_completion) {
        size_t remaining = (unsigned)buffer + filled_size - (unsigned)cursor;
        if (remaining == 0) {
            // TODO: blocking host request
            state.write_syncstate_(SYNCSTATE::STREAMREQ);
            while (state.syncstate_ != SYNCSTATE::CONTINUE) {
            };
            state.write_syncstate_(SYNCSTATE::RUN);
            return 0;
        }
        if (size > remaining) {
            // TODO: read remaining but also async host request
            state.write_syncstate_(SYNCSTATE::STREAMREQ);
            while (state.syncstate_ != SYNCSTATE::CONTINUE) {
            };
            state.write_syncstate_(SYNCSTATE::RUN);
            size = remaining;
        }
        wait();
        dma.push(dst_buf, cursor, size, 1);
        cursor = (void*)((unsigned)cursor + size);
        if (wait_for_completion)
            wait();
        return size;
    }

  private:
    // These are also part of `stream_descriptor`
    // Explanation is in combuf.hpp at `stream_descriptor`
    void* buffer;
    uint32_t capacity;
    int32_t offset;
    int32_t filled_size;
    // Only local
    __attribute__((aligned(8))) dma_task dma;
    int stream_id;
    void* cursor;
};

} // namespace epiphany
} // namespace bulk
