# `bulk::util::timer`

Defined in header `<bulk/util/timer.hpp>`.

```cpp
class timer;
```

Constructs a timer that immediately starts ticking.

### `bulk::util::timer::get`

```cpp
template <typename resolution = std::milli>
double get()
```

* **returns** the number of (by default) milliseconds that have passed since the construction of the timer.
