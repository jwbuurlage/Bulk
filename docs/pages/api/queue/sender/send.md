# `bulk::queue::sender::send`

```cpp
template <typename... Us>
void send(Us... args)
```

Send a message to a remote queue

## Parameters

* `args` - the content to send

## Complexity and cost

* **Cost** - `sizeof(Us...) * g`
