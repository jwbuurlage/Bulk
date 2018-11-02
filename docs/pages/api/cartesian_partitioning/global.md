# `bulk::cartesian_partitioning::global`

```cpp
using multi_partitioning<D, G>::global;
virtual int global(int g, int u, int i); // overload
```

Convert a local index to a global one (overload).

## Parameters

- `g`: the axis for which to compute the index
- `u`: the `g`th component of the local multi rank
- `i`: the `g`th component of the local index

## Return value

- `int`: the `g`th component of the global index

## See also

- [`partitioning::global`](../partitioning/global.md)
