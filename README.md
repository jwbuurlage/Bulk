Bulk
====

![](docs/pages/images/logo-square.png)

Bulk is a new interface for writing parallel programs in C++. It uses explicit
processes that all run the same program, but on different and mutually exclusive data (SPMD). This is
different from common parallel programming paradigms based on independent threads
that are performing (usually heterogeneous) tasks together with guarding mechanisms to prevent
concurrent access to shared resources. Programs written in Bulk work both for shared
memory systems, as well as for distributed memory systems (e.g. an MPI cluster).

Compared to other SPMD libraries, Bulk does away with unnecessary boilerplate code and ubiquitous pointer arithmetic that is
found in libraries based on for example MPI, or the BSPlib standard. Our BSP interface supports and encourages the use of
modern C++ features, enabling safer and more efficient distributed programming.
The flexible backend architecture ensures the portability of parallel programs written with Bulk.

About BSP
---------

The bulk synchronous parallel (BSP) programming model, is a way of writing parallel and distributed programs. BSP is the underlying model for Bulk. Instead of communicating between processors (or nodes, or cores) asynchronously, all communication is staged and resolved at fixed _synchronization points_. These synchronizations delimit so-called _supersteps_. This way of writing parallel programs has a number of advantages over competing models:

- The resulting programs are **structured**, easy to understand and maintain, and their performance and correctness can be reasoned about precisely.
- **Data races** are eliminated almost by construction, only have to follow a small set of simple rules which can be enforced at runtime.
- **Scalability** is easy to obtain, since programs are written in a SPMD fashion, and this scalability can be analyzed explicitely.
- The number of **communication mechanisms** required are very limited, roughly only distinguishing between anonymous (message passing) or _named_ communication (through distributed variables). This makes BSP based libraries very economic (you can do much with very little).
- It has a **low cost of entry**. It is easy to write _correct_ BSP programs, while it is notoriously hard to write correct asynchronous parallel programs.

The bulk synchronous communication style does mean losing some flexibility, and comes with a (usually minor) performance penalty. This tradeoff is often well worth it.

Examples
-------

Hello world!

```cpp
bulk::thread::environment env;
env.spawn(env.available_processors(), [](auto& world) {
    auto s = world.rank();
    auto p = world.active_processors();

    world.log("Hello world from processor %d / %d!", s, p);
});
```

Distributed variables are the easiest way to communicate.

```cpp
auto a = bulk::var<int>(world);
a(world.next_rank()) = s;
world.sync();
// ... a is now updated

auto b = a(world.next_rank()).get();
world.sync();
// ... b.value() is now available
```

Coarrays are convenient distributed arrays.

```cpp
auto xs = bulk::coarray<int>(world, 10);
xs(world.next_rank())[3] = s;
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

* Jan-Willem Buurlage (@jwbuurlage)
* Tom Bannink (@tombana)

License
-------

Bulk is released under the MIT license, for details see the file LICENSE.md.

Contributing
------------

We welcome contributions. Please submit pull requests against the develop branch.

If you have any issues, questions or remarks; either open an issue on GitHub, or
join us in the `##bulk` IRC channel on `irc.freenode.net`.
