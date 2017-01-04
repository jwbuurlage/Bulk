#pragma once

// FIXME: The queue system has been changed. This class is 'old' code.
// The new system can be found in mpi/messages.hpp or cpp/messages.hpp

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
     * A basic iterator used for iterating through the messages.
     */
    class message_iterator {};

    /**
     * This helper class adds syntactic sugar to the message passing interface
     * In particular it allows the programmer to write
     *
     *     q(pid).send(tag, content);
     */
    class sender {
        /**
         * Add a message to a (remote) queue
         *
         * \param tag the tag to attach to the message
         * \param content the content of the message
         */
        void send(Tag tag, Content content);

       private:
        sender(queue& q, int t);
    };

    /**
     * Construct a message queue and register it with world
     */
    queue(bulk::world& world) : world_(world) {}
    ~queue();

    /**
     * Obtain a sender object to a remote queue
     */
    auto operator()(int t);

    /**
     * Obtain an iterator to the begin of the local queue
     */
    message_iterator begin();

    /**
     * Obtain an iterator to the end of the local queue
     */
    message_iterator end();

    /**
     * Retrieve the world to which this var is registed.
     *
     * \returns a reference to the world of the var
     */
    bulk::world& world() { return world_; }
   private:
    bulk::world& world_;
};

}  // namespace bulk
