#pragma once

#include <bulk/world.hpp>
#include <memory>
#include <vector>

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
    /** a tag to attach to the message */
    Tag tag;
    /** the content of the message */
    Content content;
};

// queue::impl subclasses queue_base
class queue_base {
  public:
    queue_base(){};
    virtual ~queue_base(){};

    // These are called by world during a sync
    // It resizes an internal buffer and returns a pointer to it
    virtual void* get_buffer_(int size_in_bytes) = 0;

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
     * This helper class adds syntactic sugar to the message passing interface
     * In particular it allows the programmer to write
     *
     *     q(pid).send(tag, content);
     */
    class sender {
       public:
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
     */
    queue(bulk::world& world) {
        impl_ = std::make_unique<impl>(world);
        world.barrier();
    }
    ~queue() { impl_->world_.barrier(); }

    // No copies
    queue(queue<Tag, Content>& other) = delete;
    void operator=(queue<Tag, Content>& other) = delete;

    /**
      * Move constructor: move from one queue to a new one
      */
    queue(queue<Tag, Content>&& other) { impl_ = std::move(other.impl_); }

    /**
     * Move assignment: move from one queue to an existing one
     */
    void operator=(queue<Tag, Content>&& other) {
        impl_ = std::move(other.impl_);
    }

    /**
     * Obtain a sender object to a remote queue
     */
    auto operator()(int t) { return sender(*this, t); }

    bool empty() { return impl_->data_.empty(); }

    /**
     * Obtain an iterator to the begin of the local queue
     */
    auto begin() {
        return impl_->data_.begin();
    }

    /**
     * Obtain an iterator to the end of the local queue
     */
    auto end() {
        return impl_->data_.end();
    }

    /**
     * Retrieve the world to which this queue is registed.
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

         std::vector<message<Tag, Content>> data_;
         bulk::world& world_;
         int id_;
     };
     std::unique_ptr<impl> impl_;

     friend sender;
};

}  // namespace bulk
