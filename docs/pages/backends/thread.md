# `<thread>`

This backend uses the shared-memory parallelism provided by the standard C++
library `<thread>`. To use it on Linux, link against the `pthread` library.

There are two supported barrier implementations, a mutex-based barrier and
the (default) spinlock-based barrier. The mutex barrier is more energy
efficient, while the spinlock barrier is significantly faster. Choosing
the barrier can be done by passing a template argument to 
`bulk::thread::environment_`, while `bulk::thread::environment` is a
non-template type alias to the environment using the default barrier
implementation.

```cpp
// default (with spinning barrier)
auto env = bulk::thread::environment; 

// explicit (with spinning barrier)
auto env = bulk::thread::environment_<bulk::thread::spinning_barrier>; 

// explicit (with mutex barrier)
auto env = bulk::thread::environment_<bulk::thread::mutex_barrier>; 
```
