#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <bulk/messages.hpp>
#include <bulk/world.hpp>
#include <mpi.h>

#include "memory_buffer.hpp"

namespace bulk::mpi {

enum class message_t : int { put, get_request, get_response, send };

enum class put_t : int { single, multiple };

enum class get_t : int { request_single, request_multiple };

// different approach
// -> buffers should be some kind of memory pool
//
// put buffer
// -> its a (large) binary data 'file', containing a header and data and so on
// -> there is one for each remote (and local, treated differently) processor
//
// get buffer
// -> two-phase protocol, request sends at sync, then the buffer is prepared and
// sent
//
// message buffer
// -> same as put basically
//
// want writers and readers for these formats
// maybe reuse some of the ideas for the zmq server
// this should simplify implementation somewhat
// and limit the number of messages going around
//
// do we want one-sided communication? can make a test implementation based on
// MPI windows later to compare
//
// - [x] define a memory buffer object
// - [x] define readers and writers to this object for the different formats:
//   * [x] put
//   * [x] get_request
//   * [x] get_response
//   * [ ] message
// - [ ] Try to extract 'readers' and 'writers', and see if we can get better
// code reuse and modularity here

class world : public bulk::world {
  public:
    world() : bulk::world() {
        MPI_Comm_size(MPI_COMM_WORLD, &active_processors_);
        MPI_Comm_rank(MPI_COMM_WORLD, &processor_id_);
        MPI_Get_processor_name(name_, &name_length_);

        put_buffers_.resize(active_processors_);
        get_request_buffers_.resize(active_processors_);
        get_response_buffers_.resize(active_processors_);
        message_buffers_.resize(active_processors_);

        ones_.resize(active_processors_);
        std::fill(ones_.begin(), ones_.end(), 1);
    }

    virtual ~world() {}

    int active_processors() const override final { return active_processors_; }
    int processor_id() const override final { return processor_id_; }

    void sync() override final {
        clear_messages_();

        int remote_puts = 0;
        int remote_get_requests = 0;
        int remote_get_responses = 0;
        int remote_messages = 0;

        std::vector<int> put_to_proc(active_processors_);
        std::vector<int> get_request_to_proc(active_processors_);
        std::vector<int> get_response_to_proc(active_processors_);
        std::vector<int> messages_to_proc(active_processors_);

        // handle puts
        send_buffers_(put_buffers_, message_t::put, put_to_proc);

        // exchange puts, implicit barrier
        MPI_Reduce_scatter(put_to_proc.data(), &remote_puts, ones_.data(),
                           MPI_INT, MPI_SUM, MPI_COMM_WORLD);

        receive_buffer_for_tag_(put_receive_buffer_, message_t::put,
                                remote_puts);
        process_put_buffer_(put_receive_buffer_);

        // handle gets
        send_buffers_(get_request_buffers_, message_t::get_request,
                      get_request_to_proc);

        // exchange gets, implicit barrier
        MPI_Reduce_scatter(get_request_to_proc.data(), &remote_get_requests,
                           ones_.data(), MPI_INT, MPI_SUM, MPI_COMM_WORLD);

        receive_buffer_for_tag_(get_request_buffer_, message_t::get_request,
                                remote_get_requests);
        send_get_responses_(get_request_buffer_, get_response_to_proc);

        // exchange gets, implicit barrier
        MPI_Reduce_scatter(get_response_to_proc.data(), &remote_get_responses,
                           ones_.data(), MPI_INT, MPI_SUM, MPI_COMM_WORLD);

        receive_buffer_for_tag_(get_response_buffer_, message_t::get_response,
                                remote_get_responses);
        process_get_responses_(get_response_buffer_);

        // handle messages
        send_buffers_(message_buffers_, message_t::send, messages_to_proc);
        MPI_Reduce_scatter(messages_to_proc.data(), &remote_messages,
                           ones_.data(), MPI_INT, MPI_SUM, MPI_COMM_WORLD);
        receive_buffer_for_tag_(message_buffer_, message_t::send,
                                remote_messages);
        process_messages_(message_buffer_);

        barrier();
    }

    void barrier() override final { MPI_Barrier(MPI_COMM_WORLD); }

    void abort() override final {}

  protected:
    // Returns the id of the registered location
    int register_location_(void* location,
                           [[maybe_unused]] size_t size) override final {
        auto idx = 0u;
        for (; idx < locations_.size(); ++idx) {
            if (locations_[idx] == nullptr) {
                break;
            }
        }
        if (idx == locations_.size()) {
            locations_.push_back(nullptr);
        }
        locations_[idx] = location;
        return idx;
    }

    void unregister_location_(int id) override final {
        locations_[id] = nullptr;
    }

    void put_(int processor, const void* value, size_t size,
              int var_id) override final {
        put_buffers_[processor] << put_t::single;
        put_buffers_[processor] << var_id;
        put_buffers_[processor] << size;
        put_buffers_[processor].push(size, value);
    }

    // Size is per element
    void put_(int processor, const void* values, size_t size, int var_id,
              size_t offset, size_t count) override final {
        put_buffers_[processor] << put_t::multiple;
        put_buffers_[processor] << var_id;
        size_t total_offset = offset * size;
        put_buffers_[processor] << total_offset;
        put_buffers_[processor] << (size_t)(size * count);
        put_buffers_[processor].push(size * count, values);
    }

    void get_(int processor, int var_id, size_t size,
              void* target) override final {
        get_request_buffers_[processor] << get_t::request_single;
        get_request_buffers_[processor] << var_id;
        get_request_buffers_[processor] << size;
        get_request_buffers_[processor] << target;
        get_request_buffers_[processor] << processor_id_;
    }

    // Size is per element
    void get_(int processor, int var_id, size_t size, void* target,
              size_t offset, size_t count) override final {
        get_request_buffers_[processor] << get_t::request_multiple;
        get_request_buffers_[processor] << var_id;
        size_t total_offset = offset * size;
        get_request_buffers_[processor] << total_offset;
        get_request_buffers_[processor] << (size_t)(size * count);
        get_request_buffers_[processor] << target;
        get_request_buffers_[processor] << processor_id_;
    }

    // Messages
    int register_queue_(queue_base* q) override {
        auto idx = 0u;
        for (; idx < queues_.size(); ++idx) {
            if (queues_[idx] == nullptr) {
                break;
            }
        }
        if (idx == queues_.size()) {
            queues_.push_back(nullptr);
        }
        queues_[idx] = q;
        return idx;
    }

    void unregister_queue_(int id) override { queues_[id] = nullptr; }

    // data consists of both tag and content. size is total size.
    void send_(int processor, int queue_id, const void* data,
               size_t size) override {
        message_buffers_[processor] << queue_id;
        message_buffers_[processor] << size;
        message_buffers_[processor].push(size, data);
    }

    void log_(std::string message) override final {
        printf("$%i (%s): %s\n", processor_id_, name_, message.c_str());
    }

  private:
    bool send_buffer_(memory_buffer& buf, message_t tag, int processor) {
        // start sending put buffers
        if (buf.size() > 0) {
            MPI_Request req;
            MPI_Isend(buf.data(), buf.size(), MPI_BYTE, processor,
                      static_cast<int>(tag), MPI_COMM_WORLD, &req);
            buf.clear();

            return true;
        }
        return false;
    }

    void send_buffers_(std::vector<memory_buffer>& buffers, message_t tag,
                       std::vector<int>& got_sent) {
        for (int t = 0; t < active_processors_; ++t) {
            got_sent[t] = send_buffer_(buffers[t], tag, t);
        }
    }

    void receive_buffer_for_tag_(memory_buffer& buf, message_t tag, int count) {
        while (count--) {
            // getting buffers.. reuse membufs
            MPI_Status status = {};
            MPI_Probe(MPI_ANY_SOURCE, static_cast<int>(tag), MPI_COMM_WORLD,
                      &status);

            // count size of incoming message
            int incoming_size = 0;
            MPI_Get_count(&status, MPI_BYTE, &incoming_size);

            buf.ensure_room(incoming_size);

            MPI_Recv(buf.buffer(), incoming_size, MPI_BYTE, status.MPI_SOURCE,
                     static_cast<int>(tag), MPI_COMM_WORLD, &status);

            buf.update(incoming_size);
        }
    }

    void process_put_buffer_(memory_buffer& buf) {
        auto reader = buf.reader();
        while (!reader.empty()) {
            int var_id = 0;
            size_t offset = 0;
            size_t size = 0;

            put_t put_type;
            reader >> put_type;

            switch (put_type) {
            case put_t::single: {
                reader >> var_id;
                reader >> size;
                reader.copy(size, locations_[var_id]);
                break;
            }

            case put_t::multiple: {
                reader >> var_id;
                reader >> offset;
                reader >> size;

                auto write_location = (char*)locations_[var_id] + offset;
                reader.copy(size, write_location);

                break;
            }

            default: {
                log("ERROR: Unknown type of put message");
                abort();
                break;
            }
            }
        }
        buf.clear();
    }

    void send_get_responses_(memory_buffer& buf, std::vector<int>& got_sent) {
        auto reader = buf.reader();
        while (!reader.empty()) {
            int var_id = 0;
            void* target = 0;
            int processor = 0;
            size_t offset = 0;
            size_t size = 0;

            get_t get_type;
            reader >> get_type;

            switch (get_type) {
            case get_t::request_single: {
                reader >> var_id;
                reader >> size;
                reader >> target;
                reader >> processor;

                get_response_buffers_[processor] << target;
                get_response_buffers_[processor] << size;
                get_response_buffers_[processor].push(size, locations_[var_id]);

                break;
            }

            case get_t::request_multiple: {
                reader >> var_id;
                reader >> offset;
                reader >> size;
                reader >> target;
                reader >> processor;

                get_response_buffers_[processor] << target;
                get_response_buffers_[processor] << size;
                get_response_buffers_[processor].push(
                    size, (char*)locations_[var_id] + offset);

                break;
            }

            default: {
                log("ERROR: Unknown type of get message");
                abort();
                break;
            }
            }
        }
        buf.clear();

        send_buffers_(get_response_buffers_, message_t::get_response, got_sent);
    }

    void process_get_responses_(memory_buffer& buf) {
        auto reader = buf.reader();
        while (!reader.empty()) {
            void* target = 0;
            size_t size = 0;
            reader >> target;
            reader >> size;
            reader.copy(size, target);
        }
        buf.clear();
    }

    void process_messages_(memory_buffer& buf) {
        auto reader = buf.reader();
        int queue_id;
        size_t size;

        while (!reader.empty()) {
            reader >> queue_id;
            reader >> size;
            if (!queues_[queue_id]) {
              // queue already deleted
              continue;
            }
            queues_[queue_id]->unsafe_push_back(reader.current_location());
            reader.update(size);
        }
        buf.clear();
    }

    void clear_messages_() {
        for (auto q : queues_) {
            if (q) {
                q->clear_();
            }
        }
    }

    char name_[MPI_MAX_PROCESSOR_NAME];
    int name_length_ = 0;

    int processor_id_ = 0;
    int active_processors_ = 0;

    std::vector<void*> locations_;
    std::vector<queue_base*> queues_;

    // see how many puts and gets each processor receives
    std::vector<int> ones_;

    std::vector<memory_buffer> put_buffers_;
    memory_buffer put_receive_buffer_;

    std::vector<memory_buffer> get_request_buffers_;
    memory_buffer get_request_buffer_;
    std::vector<memory_buffer> get_response_buffers_;
    memory_buffer get_response_buffer_;
    std::vector<memory_buffer> message_buffers_;
    memory_buffer message_buffer_;
};

} // namespace bulk::mpi
