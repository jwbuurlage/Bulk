#pragma once

#include <functional>
#include <memory>
#include <vector>


namespace bulk {

// FIXME: Tag / Content i.o. TTag / TContent
template <typename TTag, typename TContent>
struct message {
    TTag tag;
    TContent content;
};

template <class WorldProvider>
class world {
  public:
    template <typename TTag, typename TContent>
    using message_container =
        typename WorldProvider::template message_container_type<TTag, TContent>;

    /// Returns the total number of active processors in a spmd section
    ///
    /// \returns the number of active processors
    /// \note should only be called inside a spmd section
    int active_processors() const { return provider_.active_processors(); }

    /// Returns the local processor id
    ///
    /// \returns an integer containing the id of the local processor
    /// \note should only be called inside a spmd section
    int processor_id() const { return provider_.processor_id(); }

    /// Returns the id of the next logical processor
    ///
    /// \returns an integer containing the id of the next processor
    int next_processor() const {
        return (processor_id() + 1) % active_processors();
    }

    /// Returns the id of the previous logical processor
    ///
    /// \returns an integer containing the id of the previous processor
    int prev_processor() const {
        return (processor_id() + active_processors() - 1) % active_processors();
    }

    /// Performs a global barrier synchronization of the active processors.
    void sync() const { provider_.sync(); }

    /// Sends a message to a remote processor
    ///
    /// \param processor the id of the remote processor receiving the message
    /// \param tag a tag to attach to the message
    /// \param content the content (payload) of the message */
    template <typename TTag, typename TContent>
    void send(int processor, TTag tag, TContent content) {
        provider_.internal_send_(processor, &tag, &content, sizeof(TTag),
                                 sizeof(TContent));
    }

    /// Returns an iterable container containing the messages sent in the previous superstep.
    ///
    /// \returns an iterable message container
    template <typename TTag, typename TContent>
    message_container<TTag, TContent> messages() const {
        return message_container<TTag, TContent>();
    }

    void register_location_(void* location, size_t size) {
        provider_.register_location_(location, size);
    }
    void unregister_location_(void* location) {
        provider_.unregister_location_(location);
    }

    /// Returns the provider of the world
    ///
    /// \returns the distributed system provider
    WorldProvider& provider() { return provider_; }

  private:
    WorldProvider provider_;
};

} // namespace bulk
