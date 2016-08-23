#pragma once

#include <algorithm>
#include <vector>

#include <mpi.h>

#include <bulk/messages.hpp>

namespace bulk {
namespace mpi {

template <typename Tag, typename Content, typename World>
class queue {
   public:
    class buffer_iterator {
       public:
        buffer_iterator(queue& q, int i) : q_(q), i_(i) {}

        buffer_iterator(const buffer_iterator& other)
            : q_(other.q_), i_(other.i_) {}

        void operator=(const buffer_iterator& other) {
            q_ = other.q_;
            i_ = other.i_;
        }

        buffer_iterator& operator++() {
            ++i_;
            return *this;
        }

        buffer_iterator& operator++(int) {
            auto current = *this;
            ++(*this);
            return current;
        }

        message<Tag, Content>& operator*() {
            return ((message<Tag, Content>*)(q_.buffer_))[i_];
        }

        bool operator==(const buffer_iterator& other) const {
            return (i_ == other.i_) && (&q_ == &other.q_);
        }

        bool operator!=(const buffer_iterator& other) const {
            return !(*this == other);
        }

       private:
        queue& q_;
        int i_ = 0;
    };

    class sender {
       public:
        void push(Tag tag, Content content) { q_.send_(t_, tag, content); }

       private:
        friend queue;

        sender(queue& q, int t) : q_(q), t_(t) {}

        queue& q_;
        int t_;
    };

    queue(World& world) : world_(world) {
        id_ = world_.template register_queue_<Tag, Content>(&buffer_, &count_);
    }
    ~queue() { world_.unregister_queue_(id_); }

    auto operator()(int t) { return sender(*this, t); }


    auto begin() { return buffer_iterator(*this, 0); }
    auto end() { return buffer_iterator(*this, count_); }

   private:
    friend sender;
    friend buffer_iterator;

    void send_(int t, Tag tag, Content content) {
        bulk::send(id_, world_, t, tag, content);
    }

    int id_ = -1;

    void* buffer_ = nullptr;
    int count_ = 0;

    World& world_;
};

}  // namespace mpi
}  // namespace bulk
