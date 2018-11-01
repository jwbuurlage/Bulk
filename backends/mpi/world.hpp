#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <mpi.h>

#include <bulk/future.hpp>
#include <bulk/messages.hpp>
#include <bulk/variable.hpp>
#include <bulk/world.hpp>

#include "memory_buffer.hpp"

namespace bulk::mpi {

enum class message_t : int {
    put,
    custom_put,
    get_request,
    get_response,
    custom_get_request,
    custom_get_response,
    send_custom
};

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
//   * [x] message
// - [ ] Try to extract 'readers' and 'writers', and see if we can get better
// code reuse and modularity here

class world : public bulk::world {
  public:
    world() : bulk::world() {
        MPI_Comm_size(MPI_COMM_WORLD, &active_processors_);
        MPI_Comm_rank(MPI_COMM_WORLD, &processor_id_);
        MPI_Get_processor_name(name_, &name_length_);

        put_buffers_.resize(active_processors_);
        custom_put_buffers_.resize(active_processors_);
        get_request_buffers_.resize(active_processors_);
        get_response_buffers_.resize(active_processors_);
        custom_message_buffers_.resize(active_processors_);
        custom_get_request_buffers_.resize(active_processors_);
        custom_get_response_buffers_.resize(active_processors_);

        ones_.resize(active_processors_);
        std::fill(ones_.begin(), ones_.end(), 1);
    }

    virtual ~world() {}

    int active_processors() const override final { return active_processors_; }
    int rank() const override final { return processor_id_; }

    void sync() override final {
        clear_messages_();

        int remote_puts = 0;
        int remote_custom_puts = 0;
        int remote_get_requests = 0;
        int remote_get_responses = 0;
        int remote_custom_messages = 0;
        int remote_custom_get_requests = 0;
        int remote_custom_get_responses = 0;

        std::vector<int> put_to_proc(active_processors_);
        std::vector<int> get_request_to_proc(active_processors_);
        std::vector<int> get_response_to_proc(active_processors_);
        std::vector<int> custom_put_to_proc(active_processors_);
        std::vector<int> custom_messages_to_proc(active_processors_);
        std::vector<int> custom_get_request_to_proc(active_processors_);
        std::vector<int> custom_get_response_to_proc(active_processors_);

        // handle gets
        // exchange gets, implicit barrier
        send_buffers_(get_request_buffers_, message_t::get_request,
                      get_request_to_proc);
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

        // handle custom_gets
        // exchange gets, implicit barrier
        send_buffers_(custom_get_request_buffers_,
                      message_t::custom_get_request,
                      custom_get_request_to_proc);
        MPI_Reduce_scatter(custom_get_request_to_proc.data(),
                           &remote_custom_get_requests, ones_.data(), MPI_INT,
                           MPI_SUM, MPI_COMM_WORLD);

        receive_buffer_for_tag_(custom_get_request_buffer_,
                                message_t::custom_get_request,
                                remote_custom_get_requests);
        send_custom_get_responses_(custom_get_request_buffer_,
                                   custom_get_response_to_proc);

        // exchange gets, implicit barrier
        MPI_Reduce_scatter(custom_get_response_to_proc.data(),
                           &remote_custom_get_responses, ones_.data(), MPI_INT,
                           MPI_SUM, MPI_COMM_WORLD);

        receive_buffer_for_tag_(custom_get_response_buffer_,
                                message_t::custom_get_response,
                                remote_custom_get_responses);
        process_custom_gets_(custom_get_response_buffer_);

        // handle puts
        // exchange puts, implicit barrier
        send_buffers_(put_buffers_, message_t::put, put_to_proc);
        MPI_Reduce_scatter(put_to_proc.data(), &remote_puts, ones_.data(),
                           MPI_INT, MPI_SUM, MPI_COMM_WORLD);

        receive_buffer_for_tag_(put_receive_buffer_, message_t::put,
                                remote_puts);
        process_put_buffer_(put_receive_buffer_);

        // handle custom puts
        // exchange puts, implicit barrier
        send_buffers_(custom_put_buffers_, message_t::custom_put,
                      custom_put_to_proc);
        MPI_Reduce_scatter(custom_put_to_proc.data(), &remote_custom_puts,
                           ones_.data(), MPI_INT, MPI_SUM, MPI_COMM_WORLD);
        receive_buffer_for_tag_(custom_put_receive_buffer_,
                                message_t::custom_put, remote_custom_puts);
        process_custom_puts_(custom_put_receive_buffer_);

        // handle custom messages
        send_buffers_(custom_message_buffers_, message_t::send_custom,
                      custom_messages_to_proc);
        MPI_Reduce_scatter(custom_messages_to_proc.data(),
                           &remote_custom_messages, ones_.data(), MPI_INT,
                           MPI_SUM, MPI_COMM_WORLD);
        receive_buffer_for_tag_(custom_message_buffer_, message_t::send_custom,
                                remote_custom_messages);
        process_custom_messages_(custom_message_buffer_);

        barrier();
    }

    void barrier() override final { MPI_Barrier(MPI_COMM_WORLD); }

    void abort() override final {}

  protected:
    // Returns the id of the registered location
    int register_variable_(var_base* var) override final {
        auto idx = 0u;
        for (; idx < vars_.size(); ++idx) {
            if (vars_[idx] == nullptr) {
                break;
            }
        }
        if (idx == vars_.size()) {
            vars_.push_back(nullptr);
            locations_.push_back(nullptr);
        }
        vars_[idx] = var;
        locations_[idx] = var->location_and_size().first;
        return idx;
    }

    int register_future_(class future_base* future) override final {
        auto idx = 0u;
        for (; idx < futures_.size(); ++idx) {
            if (futures_[idx] == nullptr) {
                break;
            }
        }
        if (idx == futures_.size()) {
            futures_.push_back(nullptr);
        }
        futures_[idx] = future;
        return idx;
    }

    void unregister_variable_(int id) override final {
        vars_[id] = nullptr;
        locations_[id] = nullptr;
    }
    void unregister_future_(class future_base* future) override final {
        futures_[future->id()] = nullptr;
    }

    char* put_buffer_(int target, int var_id, size_t size) override final {
        auto& buffer = custom_put_buffers_[target];
        buffer << var_id;
        buffer << size;
        buffer.ensure_room(size);
        auto loc = buffer.buffer();
        buffer.update(size);
        return loc;
    }

    // Size is per element
    void put_(int processor, const void* values, size_t size, int var_id,
              size_t offset, size_t count) override final {
        put_buffers_[processor] << var_id;
        size_t total_offset = offset * size;
        put_buffers_[processor] << total_offset;
        put_buffers_[processor] << (size_t)(size * count);
        put_buffers_[processor].push(size * count, values);
    }

    void get_buffer_(int target, int var_id,
                     class future_base* future) override final {
        auto& buffer = custom_get_request_buffers_[target];
        buffer << processor_id_;
        buffer << var_id;
        buffer << future->id();
    }

    // Size is per element
    void get_(int processor, int var_id, size_t size, void* target,
              size_t offset, size_t count) override final {
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

    char* send_buffer_(int target, int queue_id, size_t buffer_size) override {
        auto& buffer = custom_message_buffers_[target];
        buffer << queue_id;
        buffer << buffer_size;
        buffer.ensure_room(buffer_size);
        auto loc = buffer.buffer();
        buffer.update(buffer_size);
        return loc;
    }

    void log_(std::string message) override final {
        printf("$%i (%s): %s\n", processor_id_, name_, message.c_str());
    }

  private:
    bool send_single_buffer_(memory_buffer& buf, message_t tag, int processor) {
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
            got_sent[t] = send_single_buffer_(buffers[t], tag, t);
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

            reader >> var_id;
            reader >> offset;
            reader >> size;

            auto write_location = (char*)locations_[var_id] + offset;
            reader.copy(size, write_location);
        }
        buf.clear();
    }

    void send_custom_get_responses_(memory_buffer& buf,
                                    std::vector<int>& got_sent) {
        auto reader = buf.reader();
        while (!reader.empty()) {
            int target = -1;
            int var_id = -1;
            int future_id = -1;

            reader >> target;
            reader >> var_id;
            reader >> future_id;

            auto& buffer = custom_get_response_buffers_[target];

            auto size = vars_[var_id]->serialized_size();

            buffer << future_id;
            buffer << size;
            buffer.ensure_room(size);
            auto loc = buffer.buffer();
            vars_[var_id]->serialize(loc);
            buffer.update(size);
        }
        buf.clear();

        send_buffers_(custom_get_response_buffers_,
                      message_t::custom_get_response, got_sent);
    }

    void send_get_responses_(memory_buffer& buf, std::vector<int>& got_sent) {
        auto reader = buf.reader();
        while (!reader.empty()) {
            int var_id = 0;
            void* target = 0;
            int processor = 0;
            size_t offset = 0;
            size_t size = 0;

            reader >> var_id;
            reader >> offset;
            reader >> size;
            reader >> target;
            reader >> processor;

            get_response_buffers_[processor] << target;
            get_response_buffers_[processor] << size;
            get_response_buffers_[processor].push(
                size, (char*)locations_[var_id] + offset);
        }
        buf.clear();

        send_buffers_(get_response_buffers_, message_t::get_response, got_sent);
    }

    void process_get_responses_(memory_buffer& buf) {
        auto reader = buf.reader();
        while (!reader.empty()) {
            void* target = {};
            size_t size = {};
            reader >> target;
            reader >> size;
            reader.copy(size, target);
        }
        buf.clear();
    }

    void process_custom_puts_(memory_buffer& buf) {
        auto reader = buf.reader();
        int var_id = {};
        size_t size = {};

        while (!reader.empty()) {
            reader >> var_id;
            reader >> size;
            vars_[var_id]->deserialize_put(size, reader.current_location());
            reader.update(size);
        }
        buf.clear();
    }

    void process_custom_gets_(memory_buffer& buf) {
        auto reader = buf.reader();
        int future_id = {};
        size_t size = {};

        while (!reader.empty()) {
            reader >> future_id;
            reader >> size;
            futures_[future_id]->deserialize_get(size,
                                                 reader.current_location());
            reader.update(size);
        }
        buf.clear();
    }

    void process_custom_messages_(memory_buffer& buf) {
        auto reader = buf.reader();
        int queue_id = {};
        size_t size = {};

        while (!reader.empty()) {
            reader >> queue_id;
            reader >> size;
            queues_[queue_id]->deserialize_push(size,
                                                reader.current_location());
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
    std::vector<var_base*> vars_;
    std::vector<future_base*> futures_;

    // see how many puts and gets each processor receives
    std::vector<int> ones_;

    std::vector<memory_buffer> put_buffers_;
    memory_buffer put_receive_buffer_;

    std::vector<memory_buffer> custom_put_buffers_;
    memory_buffer custom_put_receive_buffer_;

    std::vector<memory_buffer> get_request_buffers_;
    memory_buffer get_request_buffer_;
    std::vector<memory_buffer> get_response_buffers_;
    memory_buffer get_response_buffer_;

    std::vector<memory_buffer> custom_get_request_buffers_;
    memory_buffer custom_get_request_buffer_;
    std::vector<memory_buffer> custom_get_response_buffers_;
    memory_buffer custom_get_response_buffer_;

    std::vector<memory_buffer> custom_message_buffers_;
    memory_buffer custom_message_buffer_;
};

} // namespace bulk::mpi
