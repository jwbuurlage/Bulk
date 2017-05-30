# `bulk::var::operator=`

```cpp
void operator=(var<T>&& other);
```

Move assignment operator. Replaces the target variable `*this` with the source variable `other`, and invalidates the source.

## Parameters

* `other` - another variable to move away from

## Complexity and cost

* **Cost** - `l` or free (backend dependent)
