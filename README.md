# Bulk [![Build Status](https://travis-ci.org/jwbuurlage/Bulk.svg?branch=master)](https://travis-ci.org/jwbuurlage/Bulk)

![](https://raw.githubusercontent.com/jwbuurlage/Bulk/master/docs/pages/images/logo-square.png)

The bulk-synchronous parallel (BSP) paradigm is an effective framework for parallel programs.
Bulk is a modern interface for writing BSP programs in C++. Modern programming language features
allow for the implementation of safe and generic parallel algorithms
for shared-memory, distributed-memory, and hybrid systems. Our interface
should appeal to BSP programmers who want to write fast, safe, clear, and portable parallel programs.

## About BSP

In the bulk-synchronous parallel (BSP) programming model, communication between
processors (or nodes, or cores) does not happen asynchronously. Instead, all
communication is staged and resolved at fixed _synchronization points_. These
synchronizations delimit so-called _supersteps_. This way of structuring
parallel programs has a number of advantages, at the cost of limiting the
application's freedom to overlap communication and computation.

- The resulting programs are **structured**, easy to understand and maintain,
  and their performance and correctness can be reasoned about.
- **Data races** are eliminated almost by construction, because of simple rules
  which can be enforced at runtime.
- **Scalability** is straightforward to obtain. Programs are written in a SPMD
  fashion.
- There are only two types of **communication mechanisms**, _message passing_
  and _named communication (through distributed variables)_. This makes BSP
  based libraries economic: you can accomplish a lot with few building blocks.
- It has a **gentle learning curve**. It is easy to write _correct_ BSP
  programs, while it is notoriously hard to write correct asynchronous parallel
  programs.

## Code examples

Hello world!

```cpp
bulk::thread::environment env;
env.spawn(env.available_processors(), [](auto& world) {
    auto s = world.rank();
    auto p = world.active_processors();

    world.log("Hello world from processor %d / %d!", s, p);
});
```

Distributed variables are the bread and butter of communication in Bulk.

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

## Building

Bulk should work across all major Linux distributions. It requires an up-to-date compiler that supports C++20, e.g. GCC >= 11.0, or Clang >= 12.0.

### Backends

Bulk supports a number of different *backends*, allowing the programs to run in
parallel using:

- `thread` for multi-core systems using standard C++ `<thread>` threading support
- `mpi` for distributed environments using MPI

There is also a special legacy backend available for the [Epiphany
coprocessor](https://www.parallella.org/), which can be found in the `epiphany`
branch. This branch has a modified version of Bulk to support portability
between MPI, `<thread>` and the Epiphany coprocessor. See
`backends/epiphany/README.md` for more details.

### Examples

The examples in the `examples` directory work for every backend. To build them,
do the following. The backends (e.g. `thread`, `mpi`) are built optionally, just
remove or add the option if you do not require them.

    mkdir build
    cd build
    cmake ..
    make thread mpi

The examples will be compiled in the `bin/{backend}` directory, prepended with
the backend name, i.e. to run the `hello` example with the `thread` backend:

    ./bin/thread/thread_hello

### Developing on top of Bulk

The easiest way to get started using Bulk is to download the source code from
[GitHub](https://www.github.com/jwbuurlage/bulk). If you use Bulk in a project
we suggest to add Bulk as a submodule:

```bash
git submodule add https://www.github.com/jwbuurlage/bulk ext/bulk
git submodule update --init
```

If you use CMake for your project, adding Bulk as a dependency is
straightforward. For this, you can use the `bulk` and `bulk_[backend]` targets.
For example, if your CMake target is called `your_program` and it uses Bulk with
the `thread` backend, you can use the following:

```cmake
add_subdirectory("ext/bulk")
target_link_libraries(your_program bulk_thread)
```

## License

Bulk is released under the MIT license, see LICENSE.md.

## Please Cite Us

If you have used Bulk for a scientific publication, we would appreciate
citations to the following paper:

[Buurlage JW., Bannink T., Bisseling R.H. (2018) Bulk: A Modern C++ Interface for Bulk-Synchronous Parallel Programs. In: Aldinucci M., Padovani L., Torquati M. (eds) Euro-Par 2018: Parallel Processing. Euro-Par 2018. Lecture Notes in Computer Science, vol 11014. Springer, Cham](https://doi.org/10.1007/978-3-319-96983-1_37)

## Applications using Bulk

| Article      |  Code  |
|------------------|--------|
| *A projection-based partitioning method for distributed tomographic reconstruction*. SIAM PP20. [DOI](https://doi.org/10.1137/1.9781611976137.6)  | [<img src="https://github.com/favicon.ico" width="24">](https://github.com/jwbuurlage/Pleiades) |

## Authors

Bulk is developed at Centrum Wiskunde & Informatica (CWI) in Amsterdam by:

* Jan-Willem Buurlage (@jwbuurlage)
* Tom Bannink (@tombana)

Also thanks to:

* Rob Bisseling
* Sarita de Berg (@SdeBerg)

## Contributing

We welcome contributions! Please submit pull requests against the `develop` branch. For each PR:

- Describe the change in `CHANGELOG.md`
- New authors may add their name to the 'thanks to' section in `README.md`
- Format the code using `clang-format`, with the configuration found in the root of this project

If you have any issues, questions, or remarks, then please open an issue on GitHub.
