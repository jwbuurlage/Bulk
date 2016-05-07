#pragma once

#include <functional>
#include <memory>
#include <vector>


namespace bulk {

template <typename T>
class coarray;

/** @class message<TTag, TContent> */
template <typename TTag, typename TContent>
struct message {
    TTag tag;
    TContent content;
};

template <template <typename> typename var, template <typename> typename array,
          template <typename> typename future,
          template <typename, typename> typename message_container>
class base_hub {
  public:
    template <typename T>
    using var_type = var<T>;

    template <typename T>
    using array_type = array<T>;

    template <typename T>
    using future_type = future<T>;

    template <typename T>
    class coarray;

    template <typename T>
    class coarray_writer {
      public:
        coarray_writer(base_hub& hub, coarray<T>& parent, int t, int i)
            : hub_(hub), parent_(parent), t_(t), i_(i) {}

        void operator=(T value) {
            hub_.put<T>(t_, value, parent_.data(), i_, 1);
        }

      private:
        base_hub& hub_;
        coarray<T>& parent_;
        int t_;
        int i_;
    };

    template <typename T>
    class coarray_image {
      public:
        coarray_image(base_hub& hub, coarray<T>& parent, int t)
            : hub_(hub), parent_(parent), t_(t) {}

        coarray_writer<T> operator[](int i) {
            return coarray_writer<T>(hub_, parent_, t_, i);
        }

      private:
        base_hub& hub_;
        coarray<T>& parent_;
        int t_;
    };

    template <typename T>
    class coarray {
      public:
        coarray(base_hub& hub, int local_size) : hub_(hub) {
            data_ = hub.create_array<T>(local_size);
        }

        coarray_image<T> operator()(int t) {
            return coarray_image<T>(hub_, *this, t);
        }

        T& operator[](int i) {
            return (*data_)[i];
        }

        friend coarray_image<T>;
        friend coarray_writer<T>;

      private:
        base_hub& hub_;
        std::unique_ptr<base_hub::array_type<T>> data_;

        base_hub::array_type<T>& data() { return *data_; }
    };

    // TODO: maybe these should return unique_ptrs to handle destruction
    // properly
    template <typename T>
    var<T> create_var() {
        return var<T>();
    }

    template <typename T>
    future<T> create_future() {
        return future<T>();
    }

    template <typename T>
    std::unique_ptr<array<T>> create_array(int local_size) {
        return std::make_unique<array<T>>(local_size);
    }
    
    template <typename T>
    coarray<T> create_coarray(int size) {
        return coarray<T>(*this, size);
    }

    /** @brief Start an spmd section on a given number of processors.
      * @param processors the number of processors to run on
      * @param spmd the spmd function that gets run on each (virtual) processor
      */
    virtual void spawn(int processors, std::function<void(int, int)> spmd) = 0;

    /** @brief Obtain the total number of processors available on the system
      * @return the number of available processors */
    virtual int available_processors() = 0;

    /** @brief Obtain the total number of active processors in a spmd section
      * @return the number of active processors
      * @note should only be called inside a spmd section */
    virtual int active_processors() = 0;

    /** @brief Obtain the local processor id
      * @return an integer containing the id of the local processor
      * @note should only be called inside a spmd section */
    virtual int processor_id() = 0;

    /** @brief Obtain the id of the next logical processor
      * @return an integer containing the id of the next processor */
    virtual int next_processor() = 0;

    /** @brief Obtain the id of the previous logical processor
      * @return an integer containing the id of the previous processor */
    virtual int prev_processor() = 0;

    /** @brief Put a value into a variable held by a (remote) processor
      * @param processor the id of a remote processor holding the variable
      * @param value the new value of the variable */
    template <typename T>
    void put(int processor, T value, var<T>& variable, int offset = 0,
             int count = 1) {
        internal_put_(processor, &value, &variable.value(), sizeof(T), offset,
                      count);
    }

    template <typename T>
    void put(int processor, T value, array<T>& variable, int offset = 0,
             int count = 1) {
        internal_put_(processor, &value, variable.data(), sizeof(T), offset,
                      count);
    }

    /** @brief Put a value into a variable held by a (remote) processor
      * @param processor the id of a remote processor holding the variable
      * @param value the new value of the variable */
    template <typename T>
    future<T> get(int processor, var<T>& variable, int offset = 0,
                  int count = 1) {
        future<T> result;
        internal_get_(processor, &variable.value(), result.buffer_, sizeof(T),
                      offset, count);
        return result;
    }

    /** @brief Perform a global barrier synchronization */
    virtual void sync() = 0;

    /** @brief Send a message to a remote processor
      * @param processor the id of the remote processor receiving the message
      * @param tag a tag to attach to the message
      * @param content the content (payload) of the message */
    template <typename TTag, typename TContent>
    void send(int processor, TTag tag, TContent content) {
        internal_send_(processor, &tag, &content, sizeof(TTag),
                       sizeof(TContent));
    }

    template <typename TTag, typename TContent>
    message_container<TTag, TContent> messages() {
        return message_container<TTag, TContent>();
    }

    // convenience functions
    template <typename T, typename TFunc>
    T reduce(var<T>& variable, TFunc f, T start_value = 0) {
        T result = start_value;
        std::vector<future<T>> images(active_processors());
        for (int t = 0; t < active_processors(); ++t) {
            images[t] = get<T>(t, variable);
        }
        sync();
        for (int t = 0; t < active_processors(); ++t) {
            f(result, images[t].value());
        }
        return result;
    }

  protected:
    virtual void internal_put_(int processor, void* value, void* variable,
                               size_t size, int offset, int count) = 0;

    virtual void internal_get_(int processor, void* variable, void* target,
                               size_t size, int offset, int count) = 0;

    virtual void internal_send_(int processor, void* tag, void* content,
                                size_t tag_size, size_t content_size);
};

} // namespace bulk
