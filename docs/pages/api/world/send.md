# `bulk::world::send`

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
