# `bulk::partitioning::multi_owner`

```cpp
int owner(index_type<D> xs) override;
virtual index_type<G> multi_owner(index_type<D> xs) // overload
```

Returns the multi rank of the processor that owns the element at a given index.

## Parameters

- `xs`: the index in the data space

## Return value

- `index_type<G>`: the multi rank of the processor that owns `xs`

## See also

- [`partitioning::owner`](../partitioning/owner.md)
