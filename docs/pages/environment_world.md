Environment and world
=====================

In the upcoming sections we will get you started with programming in Bulk. Two important concepts are that of an environment and a world.

In these code examples, we will often use the short-hand `s` for the _local processor id_ which is referred to as its _rank_, and `p` for the _active number of processors_, and we will not always define these variables explicitly.

Parallel environments
---------------------

A program runs in some parallel environment. For example, this
environment could be an MPI cluster, a many-core co-processor, or simply
threads on a multi-core computer. This environment is accessed within
the program through a [bulk::environment](/api/environment/) object.
This object is specialized for each *backend*, which is an
implementation of the lower-level communication that reflects the actual
environment. For example, to setup an environment on an MPI cluster, we
would write:

```cpp
#include <bulk/bulk.hpp>
#include <bulk/backends/mpi/mpi.hpp>

int main() {
    bulk::mpi::environment env;
}
```

For a list of default providers, consult the *backends* section of this
documentation.

This environment object contains information on the parallel system, for
example we can request the number of processors that are available.

We note that throughout this documentation (and in the library),
**processor** is a general term for the entity that executes the SPMD
section (more on this later) and communicates with other processors --
this can be an MPI node, a core, or a thread, depending on the backend
that is used, but they are all treated in the same manner.

```cpp
auto processor_count = env.available_processors();
```

This information can be used to *spawn* the program on the right amount
of processors. Programs written in Bulk follow the **SPMD** (Single
Program Multiple Data) paradigm. This means that each processor executes
the same code, but has its own (local) data that it manipulates. In
Bulk, the SPMD section is a *function object*. This can be a C++ lambda,
a `std::function`, or a C function pointer. In this
documentation we will use lambda functions for our examples.

This function will run on each processor, and should take three
arguments. The first, contains the *world* object, which we will
describe in detail in the next section. The SPMD section is executed
in the following way:

```cpp
env.spawn(env.available_processors(), [](auto& world) {
    auto s = world.rank();
    auto p = world.available_processors();
    world.log("Hello world from processor %d / %d!", s, p);
}
```

The `spawn` function takes two arguments. The first is the
total number of processors to run the SPMD section on, here we simply
use all the processors that are available. The second is the SPMD
function itself, that is run on the given number of processors.

The world of a processor
------------------------

<center>
![image](images/environment.png)
</center>

Each processor can communicate to other processors using the *world*
object of type [bulk::world](/api/world/). The world object contains
some information on the specifics of the SPMD section, such as the number of
processors executing the section, and its identifier (as we have seen,
these are also provided as arguments for programmer convenience). We can
also obtain indices of the neighbouring processors:

```cpp
auto next = world.next_rank();
auto previous = world.prev_rank();
```

The next and previous processor can also be computed manually using:

```cpp
next = (s + 1) % p;
previous = (s + p - 1) % p;
```

However, we would suggest using the appropriate methods of world to
increase readability. Another important mechanism exposed through the
world object is the ability to perform a *bulk synchronization*, which
is the cornerstone of programs written in BSP style:

```cpp
world.sync();
```

We will see the specific uses of bulk synchronization in the upcoming
sections.
