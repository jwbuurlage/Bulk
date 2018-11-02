# `bulk::partitioning::global`

```cpp
index_type<D> global(index_type<D> xs, int processor);
```

Convert a local index to a global one.

## Parameters

- `processor`: the local rank
- `xs`: the local index

## Return value

- `index_type<D>`: the global index
