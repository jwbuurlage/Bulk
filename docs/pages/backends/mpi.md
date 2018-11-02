# MPI

If you choose to use the MPI backend, then simply include the Bulk headers, and
set up your project build as though it is an MPI project. We suggest to use the
OpenMPI implementation, since is the implementation we test against.

## FAQ

- On Fedora 28 using GCC 8 or later, you may receive a error `-Werror=cast-function-type`.
  This is due to OpenMPI loading the C++ bindings, which Bulk does not use. You can circumvent
  this issue by generating the build files with the following.

```bash
cmake .. -DCMAKE_CXX_FLAGS="-DOMPI_SKIP_MPICXX"
```
