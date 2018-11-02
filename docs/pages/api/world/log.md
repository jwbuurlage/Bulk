# `bulk::world::log`

```cpp
template <typename... Ts>;
void log(const char* format, const Ts&... ts);
```

Print output for debugging, shown at the next synchronization, using `printf`
style formatting syntax.
