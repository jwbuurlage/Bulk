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
template <typename Tag, typename Content, typename World>
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
    queue(World& world);
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
};

/**
 * Sends a message to a remote processor
 *
 * \param processor the id of the remote processor receiving the message
 * \param tag a tag to attach to the message
 * \param content the content (payload) of the message
 */
template <typename Tag, typename Content, typename World>
void send(int queue_id, World& world, int processor, Tag tag, Content content) {
    world.implementation().template internal_send_<Tag, Content>(
        queue_id, processor, tag, content);
}

/**
 * Retrieve an iterable container containing the messages sent in the previous
 * superstep.
 *
 * \returns an iterable message container
 */
template <typename Tag, typename Content, typename World>
typename World::template message_container<Tag, Content> messages(World world) {
    return typename World::template message_container<Tag, Content>();
}

}  // namespace bulk
