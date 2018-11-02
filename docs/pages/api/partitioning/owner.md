# `bulk::partitioning::owner`

```cpp
virtual int owner(index_type<D> xs);
```

Returns the rank of the processor that owns the element at a given index.

## Parameters

- `xs`: the index in the data space

## Return value

- `int`: the rank of the processor that owns `xs`
