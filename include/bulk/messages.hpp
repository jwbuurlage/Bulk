#pragma once

#include <cstddef>
#include <memory>
#include <tuple>
#include <vector>

#include "world.hpp"

/**
 * \file messages.hpp
 *
 * This header defines message-passing support.
 */

namespace bulk {

/**
 * This object contains a message for or from another processor.
 */

// Partial specialization of alias templates is not allowed, so we need this
// indirection
template <typename T, typename Enable, typename... Ts>
struct message_t;

template <typename T, typename... Ts>
struct message_t<T, typename std::enable_if_t<(sizeof...(Ts) > 0)>, Ts...> {
    std::tuple<T, Ts...> content;
};

template <typename T, typename... Ts>
struct message_t<T, typename std::enable_if_t<(sizeof...(Ts) == 0)>, Ts...> {
    T content;
};

template <typename T, typename... Ts>
struct message : public message_t<T, void, Ts...> {};

// queue::impl subclasses queue_base
// The reason that this is seperate is:
// queue::impl has template Tag,Content
// whereas we need to access this object
// in a virtual function in `world`.
class queue_base {
  public:
    queue_base(){};
    virtual ~queue_base(){};

    // These are called by world during a sync
    // It resizes an internal buffer and returns a pointer to it
    virtual void* get_buffer_(int size_in_bytes) = 0;
    virtual void clear_() = 0;

    virtual void unsafe_push_back(void* msg) = 0;
};

/**
 * The queue inferface is used for sending and receiving messages.
 *
 * \tparam Tag the type to use for the message tag
 * \tparam Content the type to use for the message content
 */
template <typename T, typename... Ts>
class queue {
  public:
    using M = message<T, Ts...>;

    /**
     * A queue is a mailbox for messages of a given type.
     * They allow a convenient syntax for message passing:
     *
     *     q(processor).send(tag, content);
     */
    class sender {
      public:
        /** Send a message over the queue. */
        template <typename... Us>
        void send(Us... args) {
            M msg;
            msg.content = {args...};
            q_.impl_->send_(t_, msg);
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
    auto begin() { return impl_->data_.begin(); }

    /**
     * Get an iterator to the end of the local queue
     */
    auto end() { return impl_->data_.end(); }

    /**
     * Get the number of messages in the local queue.
     */
    std::size_t size() { return impl_->data_.size(); }

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

        void send_(int t, M m) {
            world_.send_(t, id_, &m.content, sizeof(m.content));
        }

        void* get_buffer_(int size_in_bytes) override {
            data_.resize(size_in_bytes / sizeof(M::content));
            return &data_[0];
        }

        void unsafe_push_back(void* msg) override {
            data_.push_back(*static_cast<decltype(M::content)*>(msg));
        }

        void clear_() override { data_.clear(); }

        std::vector<decltype(M::content)> data_;
        bulk::world& world_;
        int id_;
    };
    std::unique_ptr<impl> impl_;

    friend sender;
};

} // namespace bulk
