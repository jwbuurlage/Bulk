# `bulk::foldl`

```cpp
template <typename T, typename Func, typename S = T>
S foldl(var<T>& x, Func f, S start_value = {}) // (1)

template <typename T, typename Func, typename S = T>
S foldl(coarray<T>& xs, Func f, S start_value = {}) // (2)
```

Perform a left-fold over a distributed variable (1) or coarray (2).

## Template parameters

* `T` - the value type of the variable/array
* `S` - the value type of the accumulator
* `Func` - type of a binary function of the form `(S&, T) -> void` that modifies its first parameter, and is used in the fold

## Parameters

* `x` - the distributed variable
* `xs` - the coarray
* `f` - the folding function
* `start_value` - the (optional) initial value of the accumulator

## Complexity and cost

- **Cost**:
    - (1) `costof(f) * p + sizeof(T) * p * g + l`
    - (2) `costof(f) * (p + X) + sizeof(T) * p * g + l`


# `bulk::foldl_each`

```cpp
template <typename T, typename Func, typename S = T>
std::vector<T> foldl_each(coarray<T>& xs, Func f, S start_value = {})
```

Perform a left-fold over each element of a coarray.

## Template parameters

* `T` - the value type of the variable/array
* `S` - the value type of the accumulator
* `Func` - type of a binary function of the form `(S&, T) -> void` that modifies its first parameter, and is used in the fold

## Parameters

* `xs` - the coarray. Required the same local size on each processors, or an empty array is returned.
* `f` - the folding function
* `start_value` - the (optional) initial value of the accumulator

## Result

The $i$th element of the result holds a left fold of `f` over the $i$th element of each local image of the coarray `xs`.

## Complexity and cost

- **Cost**: `costof(f) * p + sizeof(T) * xs.size() * p * g + l`

# `bulk::max`

```cpp
template <typename T>
T max(bulk::world& world, T t); // (1)

template <typename T>
T max(bulk::var<T>& x); // (2)

template <typename T>
T max(bulk::coarray<T>& xs); // (3)
```

Compute the global maximum of local values.

## Template parameters

* `T` - the value type of the value/variable/array

## Parameters

* `t` - the value
* `x` - the distributed variable
* `xs` - the coarray

## Complexity and cost

- **Cost**:
    - (1), (2) `p + sizeof(T) * p * g + l`
    - (3) `xs.size() * (xs.size() * p + sizeof(T) * p * g) + l`

# `bulk::min`

```cpp
template <typename T>
T min(bulk::world& world, T t); // (1)

template <typename T>
T min(bulk::var<T>& x); // (2)

template <typename T>
T min(bulk::coarray<T>& xs); // (3)
```

Compute the global minimum of local values.

## Template parameters

* `T` - the value type of the value/variable/array

## Parameters

* `t` - the value
* `x` - the distributed variable
* `xs` - the coarray

## Complexity and cost

- **Cost**:
    - (1), (2) `p + sizeof(T) * p * g + l`
    - (3) `xs.size() * (xs.size() * p + sizeof(T) * p * g) + l`

# `bulk::sum`

```cpp
template <typename T>
T sum(bulk::world& world, T t); // (1)

template <typename T>
T sum(bulk::var<T>& x); // (2)

template <typename T>
T sum(bulk::coarray<T>& xs); // (3)
```

Compute the global sum of local values.

## Template parameters

* `T` - the value type of the value/variable/array

## Parameters

* `t` - the value
* `x` - the distributed variable
* `xs` - the coarray

## Complexity and cost

- **Cost**:
    - (1), (2) `p + sizeof(T) * p * g + l`
    - (3) `xs.size() * (xs.size() * p + sizeof(T) * p * g) + l`


# `bulk::product`

```cpp
template <typename T>
T product(bulk::world& world, T t); // (1)

template <typename T>
T product(bulk::var<T>& x); // (2)

template <typename T>
T product(bulk::coarray<T>& xs); // (3)
```

Compute the global product of local values.

## Template parameters

* `T` - the value type of the value/variable/array

## Parameters

* `t` - the value
* `x` - the distributed variable
* `xs` - the coarray

## Complexity and cost

- **Cost**:
    - (1), (2) `p + sizeof(T) * p * g + l`
    - (3) `xs.size() * (xs.size() * p + sizeof(T) * p * g) + l`




