namespace bulk {

void spawn(int processors, std::function<void(int, int)>);

int available_processors();
int active_processors();
int processor_id();

int next_processor();
int prev_processor();

template <typename T>
struct var {
    T value;
};

template <typename T>
struct future {
    T value;
};

template <typename T>
void put(int processor, T value, var<T> variable);

template <typename T>
future<T> get(int processor, var<T> variable);

void sync();

template <typename TTag, typename TContent>
void send(int processor, TTag tag, TContent content);

template <typename TTag, typename TContent>
struct message {
    TTag tag;
    TContent content
}

template <typename TTag, typename TContent>
class message_iterator : std::forward_iterator<message<TTag, TContent>> {
};

template <typename TTag, typename TContent>
class messages {
  public:
    message_iterator<TTag, TContent> begin();
    message_iterator<TTag, TContent> end();
};

template <typename TTag, typename TContent>
message_container<TTag, TContent> messages();
 
} // namespace bulk
