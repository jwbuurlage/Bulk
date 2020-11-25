# `bulk::world::sync`

```cpp
void sync(bool clear_queues = true) const;
```

Performs a global barrier synchronization of the active processors, resolving any outstanding communication.

By default, this will clear the message queues. To retain messages currently
stored in queues, set `clear_queues` to `false`.

## Complexity and cost

* **Cost** - `l`
