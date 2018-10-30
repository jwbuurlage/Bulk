The easiest way to get started using Bulk is to download the source code from [GitHub](https://www.github.com/jwbuurlage/bulk). If you use Bulk in a project we suggest to add Bulk as a submodule, since it is in active development.

Bulk requires an up-to-date compiler, that supports C++17, e.g. GCC >= 7.0, or Clang >= 4.0. Currently we only actively support Linux, but we avoid platform specific code in the library, so building on other platforms should be possible.

Bulk supports a number of different *backends*, allowing the programs to run in parallel using:

- `thread` for multi-core systems using standard C++ `<thread>` threading support
- `mpi` for distributed environments using MPI. We suggest to use the OpenMPI implementation, since is the implementation we test against.

The examples in the `examples` directory work for every backend. They are built separately for each backend. The backends (e.g. `thread`, `mpi`) are built optionally, just remove or add the option if you do not require them.

    mkdir build
    cd build
    cmake ..
    make thread mpi

The examples will be compiled in the `bin/{backend}` directory, prepended with the backend name, i.e. to run the `hello` example with the `thread` backend:

    ./bin/thread/thread_hello

Using Bulk in a project
-----------------------

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

If you do not use CMake, you can use the Bulk interface as a header-only
library. Simply add `ext/bulk/include` as an include directory. Depending on the
backend, you may have to link against addition libraries. See the backend's
documentation for more information.
