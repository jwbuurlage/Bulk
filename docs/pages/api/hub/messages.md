# `bulk::hub::messages`

```cpp
template <typename Tag, typename Content>
message_container<Tag, Content> messages()
```

Returns an iterable container containing the messages sent in the previous superstep.

## Return value

A `message_container<Tag, Content>` representing the message inbox.

## Example

```cpp
#include <iostream>

#include <bulk/bulk.hpp>
#include <bulk/bsp/bulk.hpp>


int main() {
    auto hub = bulk::hub<bulk::bsp::provider>();

    hub.spawn(hub.available_processors(), [&hub](int s, int p) {
        for (int t = 0; t < p; ++t) {
            hub.send<int, int>(t, s, s);
        }

        hub.sync();

        if (s == 0) {
            for (auto message : hub.messages<int, int>()) {
                std::cout << message.tag << ", " << message.content << std::endl;
            }
        }
    });

    return 0;
}
```
