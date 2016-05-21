#pragma once

/**
 * \file world.hpp
 *
 * This objects encodes the world of a processor and its place within it.
 */

#include <functional>
#include <memory>
#include <vector>


namespace bulk {

/**
 * This object contains a message for or from another processor.
 */
template <typename Tag, typename Content>
struct message {
    Tag tag;
    Content content;
};

/**
 * This objects encodes the world of a processor and its place within it.
 */
template <class WorldProvider>
class world {
  public:
    template <typename Tag, typename Content>
    using message_container =
        typename WorldProvider::template message_container_type<Tag, Content>;

    template <typename T>
    using var_type = typename WorldProvider::template var_type<T>;

    template <typename T>
    using future_type = typename WorldProvider::template future_type<T>;

    template <typename T>
    using coarray_type = typename WorldProvider::template coarray_type<T>;

    template <typename T>
    using array_type = typename WorldProvider::template array_type<T>;

    /**
     * Retrieve the total number of active processors in a spmd section
     *
     * \returns the number of active processors
     */
    int active_processors() const { return provider_.active_processors(); }

    /**
     * Retrieve the local processor id
     *
     * \returns an integer containing the id of the local processor
     */
    int processor_id() const { return provider_.processor_id(); }

    /**
     * Retrieve the id of the next logical processor
     *
     * \returns an integer containing the id of the next processor
     */
    int next_processor() const {
        return (processor_id() + 1) % active_processors();
    }

    /**
     * Retrieve the id of the previous logical processor
     *
     * \returns an integer containing the id of the previous processor
     */
    int prev_processor() const {
        return (processor_id() + active_processors() - 1) % active_processors();
    }

    /**
     * Performs a global barrier synchronization of the active processors.
     */
    void sync() { provider_.sync(); }

    /**
     * Sends a message to a remote processor
     *
     * \param processor the id of the remote processor receiving the message
     * \param tag a tag to attach to the message
     * \param content the content (payload) of the message
     */
    template <typename Tag, typename Content>
    void send(int processor, Tag tag, Content content) {
        provider_.internal_send_(processor, &tag, &content, sizeof(Tag),
                                 sizeof(Content));
    }

    /**
     * Retrieve an iterable container containing the messages sent in the previous superstep.
     *
     * \returns an iterable message container
     */
    template <typename Tag, typename Content>
    message_container<Tag, Content> messages() const {
        return message_container<Tag, Content>();
    }

    void register_location_(void* location, size_t size) {
        provider_.register_location_(location, size);
    }
    void unregister_location_(void* location) {
        provider_.unregister_location_(location);
    }

    /**
     * Retrieve the provider of the world
     *
     * \returns the distributed system provider
     */
    WorldProvider& provider() { return provider_; }

  private:
    WorldProvider provider_;
};

} // namespace bulk
