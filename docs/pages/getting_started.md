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

To use Bulk in a project managed with git, add it as a submodule:

```bash
git submodule add https://www.github.com/jwbuurlage/bulk ext/bulk
git submodule update --init --remote
```

And add `ext/bulk/include` as an include directory when building.

The entire library is header only, but backends may have dependencies. See their documentation for details.
