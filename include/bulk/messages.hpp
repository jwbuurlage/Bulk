#pragma once

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
        void send(Tag tag, Content content) { q_.send_(t_, tag, content); }

       private:
        friend queue;

        sender(queue& q, int t) : q_(q), t_(t) {}

        queue& q_;
        int t_;
    };

    /**
     * Construct a message queue and register it with world
     */
    queue(bulk::world& world) : world_(world) {
        // TODO
    }
    ~queue() {
        // TODO
    }

    // No copies
    queue(queue<Tag, Content>& other) = delete;
    void operator=(queue<Tag, Content>& other) = delete;

    /**
      * Move constructor: move from one queue to a new one
      */
    queue(queue<Tag, Content>&& other) {
        // TODO
    }

    /**
     * Move assignment: move from one queue to an existing one
     */
    void operator=(queue<Tag, Content>&& other) {
        // TODO
    }

    /**
     * Obtain a sender object to a remote queue
     */
    auto operator()(int t) { return sender(*this, t); }

    /**
     * Obtain an iterator to the begin of the local queue
     */
    auto begin() {
        // TODO
        return (message<Tag,Content>*)(nullptr);
    }

    /**
     * Obtain an iterator to the end of the local queue
     */
    auto end() {
        // TODO
        return (message<Tag,Content>*)(nullptr);
    }

    /**
     * Retrieve the world to which this var is registed.
     *
     * \returns a reference to the world of the var
     */
    bulk::world& world() { return world_; }
   private:
    bulk::world& world_;

    friend sender;

    void send_(int t, Tag tag, Content content) {
        // TODO
    }
};

}  // namespace bulk
