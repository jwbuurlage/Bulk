/* For now this file is meant as an interface definition only,
 * actual implementations should use this interface.
 *
 * There are a number of reasons for this, including (lack of)
 * templated virtual functions, and specialization of classes such
 * as var<T> and future<T>.
 */

#include <functional>

namespace bulk {

/** @class var<T>
  * @note a variable must be constructed in the same superstep by each
  * processor */
template <typename T>
class var {
  public:
    var();
    ~var();

    T& value();

  private:
    /** value stored by the variable */
    T value_;
};

/** @class future<T>
  * @note a future variable is validated after the next global syncronization */
template <typename T>
class future {
  public:
    future(T* buffer_);
    ~future();

    future(future<T>& other) = delete;
    future(future<T>&& other);

    T value();

    T* buffer_;
};

class bulk_base {
    /** @brief Start an spmd section on a given number of processors.
      * @param processors the number of processors to run on
      * @param spmd the spmd function that gets run on each (virtual) processor
      */
    virtual void spawn(int processors, std::function<void()> spmd) = 0;

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
    void put(int processor, T value, var<T>& variable) {
        internal_put_(processor, value, variable, sizeof(T));
    }

    /** @brief Put a value into a variable held by a (remote) processor
      * @param processor the id of a remote processor holding the variable
      * @param value the new value of the variable */
    template <typename T>
    future<T>&& get(int processor, var<T>& variable);

    /** @brief Perform a global barrier synchronization */
    void sync();

    /** @brief Send a message to a remote processor
      * @param processor the id of the remote processor receiving the message
      * @param tag a tag to attach to the message
      * @param content the content (payload) of the message */
    template <typename TTag, typename TContent>
    void send(int processor, TTag tag, TContent content);

    /** @class message<TTag, TContent> */
    template <typename TTag, typename TContent>
    struct message {
        TTag tag;
        TContent content;
    };

    /** @class message_iterator<TTag, TContent> */
    template <typename TTag, typename TContent>
    class message_iterator
        : std::iterator<std::forward_iterator_tag, message<TTag, TContent>> {
      public:
        message_iterator() {}

        message_iterator& operator--(int);
        message_iterator& operator++(int);

        bool operator==(const message_iterator& other) const;
        bool operator!=(const message_iterator& other) const;

        message<TTag, TContent> operator*();

        message_iterator& operator--();
        message_iterator& operator++();

      private:
        int i_;
    };

    /** @class messages<TTag, TContent> */
    template <typename TTag, typename TContent>
    class messages {
      public:
        messages();

        message_iterator<TTag, TContent> begin();
        message_iterator<TTag, TContent> end();
    };

    private:
      virtual internal_put_(int processor, void* value, void* variable,
                            size_t size) = 0;
};

} // namespace bulk
