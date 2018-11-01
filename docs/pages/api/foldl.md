# `bulk::foldl`

```cpp
template <typename T, typename Func, typename S = T>
S foldl(var<T>& x, Func f, S start_value = {})
```

Perform a left-fold over a distributed variable.

## Template parameters

* `T` - the value type of the variable/array
* `S` - the value type of the accumulator
* `Func` - type of a binary function of the form `(S, T) -> S` that is used to fold

## Parameters

* `x` - the distributed variable
* `f` - the folding function
* `start_value` - the (optional) initial value of the accumulator

## Complexity and cost

- **Cost**: `costof(f) * p + sizeof(T) * p * g + l`


