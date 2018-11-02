# `bulk::cartesian_partitioning::owner`

```cpp
using multi_partitioning<D, G>::owner;
virtual int owner(int g, int i);
```

Returns the rank of the processor that owns the element at a given index.

## Parameters

- `g`: the axis for which to compute the index
- `i`: the `g`th component of the local index

## Return value

- `int`: the `g`th component of the multi rank of the owner

## See also

- [`partitioning::owner`](../partitioning/owner.md)
