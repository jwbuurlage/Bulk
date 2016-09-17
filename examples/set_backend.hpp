// Included by examples to choose a backend
// Passing -DBACKEND_MPI or -DBACKEND_EPIPHANY
// will set `provider` to that provider

#if defined BACKEND_MPI

#include <bulk/backends/mpi/mpi.hpp>
using provider = bulk::mpi::provider;

#elif defined BACKEND_EPIPHANY

#include <bulk/backends/epiphany/host.hpp>
using provider = bulk::epiphany::provider;

#else

#include <bulk/backends/cpp/cpp.hpp>
using provider = bulk::cpp::provider;

#endif
