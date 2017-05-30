# `bulk::coarray::coarray`

```cpp
coarray(world& world, int local_size); // (1)
coarray(world& world, int local_size, T default_value); // (2)
```

1. Constructs a coarray of size `p x local_size` and registers it with `world`.
2. ... In addition, also initializes the elements as `default_value`.

## Parameters

* `world` - the world this variable belongs to
* `local_size` - the size of the coarray image of the local processor
* `default_value` - the initial value of the local coarray elements

## Complexity and cost

* **Cost** - `l` or free (backend dependent)
