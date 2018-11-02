# `bulk::partitioning::local_size`

```cpp
virtual index_type<D> local_size(int processor);
```

Returns the local data space shape for a given processor.

## Parameters

- `processor`: the local rank for which to return the shape.

## Return value

- `index_type<D>`: The shape of the local data space.
