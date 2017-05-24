#pragma once

#include <utility>
#include <vector>
#include <mutex>

#include <bulk/messages.hpp>

namespace bulk {
namespace thread {

template <typename Tag, typename Content, typename World>
class queue {
   public:
    class sender {
       public:
        void send(Tag tag, Content content) { q_.send_(t_, tag, content); }

       private:
        friend queue;

        sender(queue& q, int t) : q_(q), t_(t) {}

        queue& q_;
        int t_;
    };

    queue(World& world) : world_(world) {
        // Let core 0 allocate p vectors
        auto pid = world_.processor_id();
        if (pid == 0) {
            all_values_ = new queuetype[world_.active_processors()];
            world_.implementation().set_pointer_(all_values_);
        }
        world_.implementation().barrier();
        all_values_ = world_.implementation().template get_pointer_<queuetype>();

        queuetype* my_queue = &all_values_[pid];
        sync_id_ = world_.implementation().register_sync_operation_(
            [my_queue]() { my_queue->sync(); });

        world_.implementation().barrier();
        self_value_ = &all_values_[pid];
    }
    ~queue() {
        world_.implementation().barrier();
        if (sync_id_ != -1)
            world_.implementation().unregister_sync_operation_(sync_id_);
        if (world_.processor_id() == 0 && all_values_ != 0)
            delete[] all_values_;
    }

    // No copies
    queue(queue<Tag, Content, World>& other) = delete;
    void operator=(queue<Tag, Content, World>& other) = delete;

    /**
      * Move constructor: move from one queue to a new one
      */
    queue(queue<Tag, Content, World>&& other)
        : self_value_(other.self_value_), all_values_(other.all_values_),
          sync_id_(other.sync_id_), world_(other.world_) {
        // Note that `other` will be deconstructed right away, so we
        // must make sure that it does not deallocate the buffer.
        other.all_values_ = 0;
        // Also make sure it does not unregister the sync operation
        other.sync_id_ = -1;
    }

    /**
     * Move assignment: move from one queue to an existing one
     */
    void operator=(queue<Tag, Content, World>&& other) {
        if (this != &other) {
            // Note that `other` will be deconstructed right away.
            // Unlike the move constructor above, we already have something
            // allocated ourselves.
            // One of the two should be deallocated. To avoid a memcpy
            // we swap the buffers and the other queue will deallocate our
            // old buffer.
            std::swap(self_value_, other.self_value_);
            std::swap(all_values_, other.all_values_);
            std::swap(sync_id_, other.sync_id_);
        }
    }

    auto operator()(int t) { return sender(*this, t); }

    auto begin() { return self_value_->receiving->begin(); }
    auto end() { return self_value_->receiving->end(); }

   private:
    friend sender;

    void send_(int t, Tag tag, Content content) {
        auto& q = all_values_[t];
        std::lock_guard<std::mutex> lock{q.mutex};
        q.sending->push_back(message<Tag,Content>{tag,content});
    }

    // a sync swaps the two queue buffers
    // one is for receiving, the other for sending
    // the sending queue is protected with a mutex
    struct queuetype {
        std::vector<message<Tag,Content>>* receiving;
        std::vector<message<Tag,Content>>* sending;
        std::vector<message<Tag,Content>> data1;
        std::vector<message<Tag,Content>> data2;
        std::mutex mutex; // sending mutex

        queuetype() {
            receiving = &data1;
            sending = &data2;
        }
        void sync() {
            std::swap(sending, receiving);
            sending->clear();
        }
    };

    queuetype* self_value_;
    queuetype* all_values_;
    int sync_id_;
    World& world_;
};

}  // namespace thread
}  // namespace bulk
