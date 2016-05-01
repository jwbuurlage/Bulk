#include <functional>

namespace bulk {

// Initialization and information
// ------------------------------

void spawn(int processors, std::function<void(int, int)>) { }

int available_processors() { return 0; }
int active_processors() { return 0; }
int processor_id() { return 0; }

int next_processor() { return 0; }
int prev_processor() { return 0; }


// Communication and synchronization
// ---------------------------------

template <typename T>
void put(int processor, T value, var<T> variable) { }

template <typename T>
future<T> get(int processor, var<T> variable) { }

void sync() { }


// Messages
// --------

template <typename TTag, typename TContent>
void send(int processor, TTag tag, TContent content) { }

template <typename TTag, typename TContent>
messages<TTag, TContent>::messages() { }

template <typename TTag, typename TContent>
message_iterator<TTag, TContent> messages<TTag, TContent>::begin() { }

template <typename TTag, typename TContent>
message_iterator<TTag, TContent> messages<TTag, TContent>::end() { }

// Message iterator
// --------
 
} // namespace bulk
