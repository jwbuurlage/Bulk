# `bulk::foldl`

```cpp
template <typename T, typename Func>
T foldl(var<T>& x, Func f, T start_value = 0)
```

Perform a left-fold over a distributed variable.

## Template parameters

* `T` - the value type of the variable/array
* `Func` - type of a binary function of the form `(T, T) -> T` that is used to fold

## Parameters

* `x` - the distributed variable
* `f` - the folding function
* `start_value` - the (optional) initial alue of the accumulator

## Complexity and cost

- **Cost**: `costof(f) * p + sizeof(T) * p * g + l`


