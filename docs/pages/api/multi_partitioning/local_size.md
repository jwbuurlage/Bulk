# `bulk::multi_partitioning::local_size`

```cpp
index_type<D> local_size(int processor) override;
virtual index_type<D> local_size(index_type<G> processor) // overload
```

Returns the local data space shape for a given processor (overload).

## Parameters

- `processor`: the multi rank for which to return the local shape.

## Return value

- `index_type<D>`: The shape of the local data space.

## See also

- [`partitioning::local_size`](../partitioning/local_size.md)
