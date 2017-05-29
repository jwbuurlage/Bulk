Bulk
====

Bulk is a new interface for writing parallel programs in C++ in bulk-synchronous style. The library does away with the unnecessary boilerplate and ubiquitous pointer arithmetic that is found in libraries based on for example MPI, or the BSPlib standard. Our BSP interface supports and encourages the use of modern C++ features such as smart pointers, range based for loops and anonymous functions, enabling safer and more efficient distributed programming. The flexible backend architecture ensures the portability of parallel programs written with Bulk.

Examples
-------

Hello world!

```cpp
bulk::thread::environment env;
env.spawn(env.available_processors(), [](auto& world) {
    auto s = world.processor_id();
    auto p = world.active_processors();

    world.log("Hello world from processor %d / %d\n", s, p);
});
```

Distributed variables are the easiest way to communicate.

```cpp
auto a = bulk::var<int>(world);
a(world.next_processor()) = s;
world.sync();
// ... a is now updated

auto b = a(world.next_processor()).get();
world.sync();
// ... b.value() is now available
```

Coarrays are convenient distributed arrays.

```cpp
auto xs = bulk::coarray<int>(world, 10);
xs(world.next_processor())[3] = s;
```

Message passing can be used for more flexible communication.

```cpp
auto q = bulk::queue<int, float>(world);
for (int t = 0; t < p; ++t) {
    q(t).send(s, 3.1415f);  // send (s, pi) to processor t
}
world.sync();

// messages are now available in q
for (auto [tag, content] : q) {
    world.log("%d got sent %d, %f\n", s, tag, content);
}
```

Building
--------

Bulk requires an up-to-date compiler, that supports C++17, e.g. GCC >= 7.0, or Clang >= 4.0.

Bulk supports a number of different *backends*, allowing the programs to run in parallel using:

- `thread` for multi-core systems using standard C++ `<thread>` threading support
- `mpi` for distributed environments using MPI

The examples in the `examples` directory work for every backend. To build them, do the following. The backends (e.g. `thread`, `mpi`) are built optionally, just remove or add the option if you do not require them.

    mkdir build
    cd build
    cmake ..
    make thread mpi

The examples will be compiled in the `bin/{backend}` directory, prepended with the backend name, i.e. to run the `hello` example with the `thread` backend:

    ./bin/thread/thread_hello

Authors
-------

Bulk is developed by:

* Jan-Willem Buurlage (jwbuurlage)
* Tom Bannink (tombana)

License
-------

Bulk is released under the MIT license, for details see the file LICENSE.md.

Contributing
------------

We welcome contributions. Please submit pull requests against the develop branch.
