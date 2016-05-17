# `bulk::world::messages`

```cpp
template <typename Tag, typename Content>
message_container<Tag, Content> messages()
```

Returns an iterable container containing the messages sent in the previous superstep.

## Return value

A `message_container<Tag, Content>` representing the message inbox.
