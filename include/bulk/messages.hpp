#pragma once

#include <cstddef>
#include <memory>
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
template <typename Tag, typename Content>
struct message {
    /** a tag attached to the message */
    Tag tag;
    /** the content of the message */
    Content content;
};

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
template <typename Tag, typename Content>
class queue {
  public:
    /**
     * A queue is a mailbox for messages of a given type.
     * They allow a convenient syntax for message passing:
     *
     *     q(processor).send(tag, content);
     */
    class sender {
      public:
        /** Send a message over the queue. */
        void send(Tag tag, Content content) {
            q_.impl_->send_(t_, tag, content);
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
    queue(queue<Tag, Content>& other) = delete;
    void operator=(queue<Tag, Content>& other) = delete;

    /**
      * Move a queue.
      */
    queue(queue<Tag, Content>&& other) { impl_ = std::move(other.impl_); }

    /**
     * Move a queue.
     */
    void operator=(queue<Tag, Content>&& other) {
        impl_ = std::move(other.impl_);
    }

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

        void send_(int t, Tag tag, Content content) {
            message<Tag, Content> m{tag, content};
            world_.send_(t, id_, &m, sizeof(m));
        }

        void* get_buffer_(int size_in_bytes) override {
            data_.resize(size_in_bytes / sizeof(message<Tag, Content>));
            return &data_[0];
        }

        void unsafe_push_back(void* msg) override {
            data_.push_back(*static_cast<message<Tag, Content>*>(msg));
        }

        void clear_() override { data_.clear(); }

        std::vector<message<Tag, Content>> data_;
        bulk::world& world_;
        int id_;
    };
    std::unique_ptr<impl> impl_;

    friend sender;
};

} // namespace bulk
