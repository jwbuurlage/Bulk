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
 * Sends a message to a remote processor
 *
 * \param processor the id of the remote processor receiving the message
 * \param tag a tag to attach to the message
 * \param content the content (payload) of the message
 */
template <typename Tag, typename Content, typename World>
void send(World& world, int processor, Tag tag, Content content) {
    world.implementation().internal_send_(processor, &tag, &content, sizeof(Tag),
                             sizeof(Content));
}

/**
 * Retrieve an iterable container containing the messages sent in the previous superstep.
 *
 * \returns an iterable message container
 */
template <typename Tag, typename Content, typename World>
typename World::template message_container<Tag, Content> messages(World world) {
    return typename World::template message_container<Tag, Content>();
}

} // namespace bulk
