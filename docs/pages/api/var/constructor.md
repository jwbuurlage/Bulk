# `bulk::var::var`

```cpp
var(world& world); // (1)
var(world& world, T value); // (2)
```

1. Constructs a variable and registers it with `world`. Requires `T` to be trivially constructable.
2. ... In addition, set the initial value of the local image to `value`.

## Parameters

* `world` - the world this variable belongs to
* `value` - initial value of the local image

## Complexity and cost

* **Cost** - `l` or free (backend dependent)
