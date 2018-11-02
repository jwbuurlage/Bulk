# `bulk::cartesian_partitioning::local`

```cpp
using multi_partitioning<D, G>::local;
virtual int local(int g, int i); // overload
```

Convert a global index to a local one (overload).

## Parameters

- `g`: the axis for which to compute the index
- `i`: the `g`th component of the global index

## Return value

- `int`: the `g`th component of the local index

## See also

- [`partitioning::local`](../partitioning/local.md)
