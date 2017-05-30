# `bulk::gather_all`

```cpp
template <typename T>
bulk::coarray<T> gather_all(bulk::world& world, T value)
```

This function takes a value on each processor, and gathers all of these in the local images of a coarray.

## Template parameters

* `T` - the value type

## Parameters

* `world` - the world in which we gather
* `value` - the distributed variable

## Returns

- A coarray, whose local images consists of `p` elements with on the `s`-th position the value passed by the `s`-th processor.

## Complexity and cost

- **Cost**: `sizeof(T) * p * g + l`


