# `bulk::message`

Defined in header `<bulk/hub.hpp>`.

```cpp
template <typename Tag, typename Content>
struct message;
```

`bulk::message` is a tagged message that carries a payload.


## Member objects

| **Definition**    |                                         |
|-------------------|-----------------------------------------|
| `Tag tag`         | the tag attached to the message         |
| `Content content` | the content or *payload* of the message |
