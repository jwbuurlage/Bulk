# `bulk::var::broadcast`

```cpp
void broadcast(T x);
```

Broadcasts a value to all images.

For one variable, this function should be called by a single processor in any given superstep. The broadcasted value is valid starting from the subsequent superstep.

## Parameters

* `x` - the value to broadcast

## Complexity and cost

* **Cost** - `p * g`
