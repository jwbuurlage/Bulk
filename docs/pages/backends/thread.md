# `<thread>`

This backend uses the shared-memory parallelism provided by the standard C++
library `<thread>`. To use it on Linux, link against the `pthread` library.

There are three supported barrier implementations, a mutex-based barrier, a 
spinlock-based barrier, and `std::barier`. The mutex barrier is more
energy efficient, however the spinlock barrier is significantly faster. Choosing
the barrier can be done by passing a template argument to
`bulk::thread::environment_`, while `bulk::thread::environment` is a
non-template type alias to the environment using the default barrier
implementation.

A custom barrier implementation can also be supplied. It requires a constructor
that takes a single `std::ptrdiff_t`, and a function `void wait()` to make a
thread enter the barrier.

```cpp
// default (with std::barrier)
auto env = bulk::thread::environment; 

// explicit (with spinning barrier)
auto env = bulk::thread::environment_<bulk::thread::spinning_barrier>; 

// explicit (with mutex barrier)
auto env = bulk::thread::environment_<bulk::thread::mutex_barrier>; 
```
