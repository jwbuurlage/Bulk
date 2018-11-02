# `bulk::rectangular_partitioning::origin`

```cpp
virtual index_type<D> origin(index_type<G> processor) const; // (1)
virtual index_type<D> origin(int processor) const; // (2)
```

Returns the origin of the local data.

## Parameters

- `processor`: the multi rank (1) or rank (2) for which to return the local origin.

## Return value

- `index_type<D>`: The origin of the local data space.
