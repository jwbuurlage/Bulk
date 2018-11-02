# `bulk::multi_partitioning::global`

```cpp
index_type<D> global(index_type<D> xs, int processor) override;
virtual index_type<D> global(index_type<D> xs, index_type<G> processor); // overload
```

Convert a local index to a global one (overload).

## Parameters

- `processor`: the local multi rank
- `xs`: the local index

## Return value

- `index_type<D>`: the global index

## See also

- [`partitioning::global`](../partitioning/global.md)
