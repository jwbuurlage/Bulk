# `bulk::world::log_once`

```cpp
template <typename... Ts>;
void log_once(const char* format, const Ts&... ts);
```

Print output for debugging, shown at the next synchronization, using `printf`
style formatting syntax. Only messages by the first processor get output.
