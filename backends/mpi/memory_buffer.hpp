#pragma once

#include <cstring>

namespace bulk::mpi {

class memory_buffer {
  public:
    class memory_reader {
      public:
        memory_reader(memory_buffer& buf) : buf_(buf) {}
        size_t location_ = 0;

        bool empty() { return location_ >= buf_.size(); }

        template <typename T>
        void operator>>(T& rhs) {
            rhs = *(T*)(buf_.data() + location_);
            location_ += sizeof(T);
        }

        void copy(std::size_t size, void* location) {
            memcpy(location, buf_.data() + location_, size);
            location_ += size;
        }

        char* current_location() { return buf_.data() + location_; }

        void update(std::size_t size) { location_ += size; }

      private:
        memory_buffer& buf_;
    };

    memory_buffer(std::size_t capacity) : capacity_(capacity) {
        data_ = (char*)malloc(capacity_);
    }

    /** Start with 1 kB by default */
    memory_buffer() : memory_buffer(1024) {}

    ~memory_buffer() {
        if (data_) {
            free(data_);
        }
    }

    void ensure_room(std::size_t size) {
        while (size_ + size > capacity_) {
            enlarge_();
        }
    }

    void push(std::size_t size, const void* data) {
        ensure_room(size);
        memcpy(data_ + size_, data, size);
        update(size);
    }

    template <typename T>
    void operator<<(T data) {
        auto size = sizeof(T);
        push(size, &data);
    }

    void clear() { size_ = 0; }

    char* data() { return data_; }
    char* buffer() { return data_ + size_; }
    std::size_t size() { return size_; }
    void update(std::size_t pushed) { size_ += pushed; }

    memory_reader reader() { return memory_reader(*this); }

  private:
    void enlarge_() {
        int factor = 2;
        capacity_ *= factor;
        data_ = (char*)realloc(data_, capacity_);
    }

    std::size_t capacity_ = 1024;
    std::size_t size_ = 0;
    char* data_ = nullptr;
};

} // namespace bulk::mpi
