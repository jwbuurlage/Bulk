# `bulk::hub::send`

```cpp
template <typename Tag, typename Content>
void send(int processor, Tag tag, Content content);
```

Sends a message to a remote processor.

## Template parameters

* `Tag` - the type of the tag
* `Content` - the type of the content

## Parameters

* `processor` - the id of the target processor
* `tag` - the tag to attach to the message
* `content` - the content of the message

## Complexity and cost

* **Cost** - `(sizeof(Tag) + sizeof(Content)) * g`

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
