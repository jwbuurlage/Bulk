# `bulk::var::operator T`

```cpp
operator T&();
operator const T&();
```

Obtain an implicit (const) reference to the value of the local image.

## Example

This allows for convenient syntax when working with local images, e.g.:

```cpp
auto x = bulk::var<int>(world, 5);
auto y = x + 5; // y is an int with value 10
```
