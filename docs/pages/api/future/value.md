# `bulk::future::value`

```cpp
T& value();
```

Returns a reference to the value held by the future

## Return value

A reference to the value

## Note

This becomes valid after the next global synchronisation upon the initialization of the value of the future using e.g. `bulk::get`.
