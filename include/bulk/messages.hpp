#pragma once

#include <cstddef>
#include <cstring>
#include <memory>
#include <tuple>
#include <vector>

#include "util/meta_helpers.hpp"
#include "util/serialize.hpp"
#include "world.hpp"

/**
 * \file messages.hpp
 *
 * This header defines message-passing support.
 */

namespace bulk {

using namespace bulk::meta;

// queue::impl subclasses queue_base
// The reason that this is seperate is:
// queue::impl has template Tag,Content
// whereas we need to access this object
// in a virtual function in `world`.
class queue_base {
  public:
    virtual ~queue_base() = default; 

    virtual void clear_() = 0;
    virtual void deserialize_push(size_t size, char* data) = 0;
};

/**
 * The queue inferface is used for sending and receiving messages.
 *
 * \tparam Tag the type to use for the message tag
 * \tparam Content the type to use for the message content
 */
template <typename... Ts>
class queue {
  public:
    using message_type = decltype(message<Ts...>::content);
    using iterator = typename std::vector<message_type>::iterator;

    /**
     * A queue is a mailbox for messages of a given type.
     * They allow for a convenient message passing syntax:
     *
     *     q(processor).send(content...);
     */
    class sender {
      public:
        /** Send a message over the queue. */
        void send(typename representation<Ts>::type... args) {
            q_.impl_->send_(t_, args...);
        }

      private:
        friend queue;

        sender(queue& q, int t) : q_(q), t_(t) {}

        queue& q_;
        int t_;
    };

    /**
     * Construct a message queue and register it with world
     * The world implementation can choose to perform a synchronization
     */
    queue(bulk::world& world) { impl_ = std::make_unique<impl>(world); }
    ~queue() {}

    // Disallow copies
    queue(queue& other) = delete;
    void operator=(queue& other) = delete;

    /**
     * Move a queue.
     */
    queue(queue&& other) { impl_ = std::move(other.impl_); }

    /**
     * Move a queue.
     */
    void operator=(queue&& other) { impl_ = std::move(other.impl_); }

    /**
     * Get an object with which you can send to a remote queue
     */
    auto operator()(int t) { return sender(*this, t); }

    /**
     * Get an iterator to the begin of the local queue
     */
    iterator begin() { return impl_->data_.begin(); }

    /**
     * Get an iterator to the end of the local queue
     */
    iterator end() { return impl_->data_.end(); }

    /**
     * Get the number of messages in the local queue.
     */
    size_t size() { return impl_->data_.size(); }

    /**
     * Check if the queue is empty.
     */
    bool empty() { return impl_->data_.empty(); }

    /**
     * Get a reference to the world of the queue.
     *
     * \returns a reference to the world of the queue
     */
    bulk::world& world() { return impl_->world_; }

  private:
    class impl : public queue_base {
      public:
        impl(bulk::world& world) : world_(world) {
            id_ = world.register_queue_(this);
        }
        ~impl() { world_.unregister_queue_(id_); }

        // No copies or moves
        impl(impl& other) = delete;
        impl(impl&& other) = delete;
        void operator=(impl& other) = delete;
        void operator=(impl&& other) = delete;

        void send_(int t, typename representation<Ts>::type... args) {
            bulk::detail::scale ruler;
            bulk::detail::fill(ruler, args...);
            auto target_buffer = world_.send_buffer_(t, id_, ruler.size);
            auto membuf = bulk::detail::memory_buffer_base(target_buffer);
            auto ibuf = bulk::detail::imembuf(membuf);
            bulk::detail::fill(ibuf, args...);
        }

        void deserialize_push(size_t, char* data) override {
            auto membuf = bulk::detail::memory_buffer_base(data);
            data_.push_back(message_type{});
            auto obuf = bulk::detail::omembuf(membuf);
            bulk::detail::fill(obuf, data_[data_.size() - 1]);
        }

        void clear_() override { data_.clear(); }

        std::vector<message_type> data_;
        bulk::world& world_;
        int id_;
    };
    std::unique_ptr<impl> impl_;

    friend sender;
};

} // namespace bulk
