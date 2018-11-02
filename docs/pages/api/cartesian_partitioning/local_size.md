# `bulk::multi_partitioning::local_size`

```cpp
using multi_partitioning<D, G>::local_size;
virtual int local_size(int g, int u); // overload
```

Returns the local data space shape for a given processor (overload).

## Parameters

- `g`: the axis for which to compute the index
- `u`: the `g`th component of the local multi rank

## Return value

- `int`: The `g`th component of the shape of the local data space.

## See also

- [`partitioning::local_size`](../partitioning/local_size.md)
