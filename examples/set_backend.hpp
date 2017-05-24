// Included by examples to choose a backend
// Passing -DBACKEND_MPI or -DBACKEND_EPIPHANY
// will set `environment` to that environment

#if defined BACKEND_MPI

#include <bulk/backends/mpi/mpi.hpp>
using environment = bulk::mpi::environment;

#elif defined BACKEND_EPIPHANY

#include <bulk/backends/epiphany/host.hpp>
using environment = bulk::epiphany::environment;

#elif defined BACKEND_XEONPHI

#include <bulk/backends/xeonphi/xeonphi.hpp>
using environment = bulk::xeonphi::environment;

#else

#include <bulk/backends/thread/thread.hpp>
using environment = bulk::thread::environment;

#endif
