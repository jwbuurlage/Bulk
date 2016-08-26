Bulk Project Overview
=====================

Bulk provides a common interface for parallel (and distributed) applications. At the highest level Bulk consists of a _parallel environment_. This environment provides _worlds_ to the processing nodes, which they use to communicate with eachother.

## Environment

The environment depends on a _provider_, which is the implementation of the low level functions that power the rest of the interface. We provide a number of such backends (e.g. MPI, BSP, native C++ threads, Epiphany). The environment is initialized as follows:

```
auto env = bulk::environment<bulk::mpi::provider>();
```

Here the environment takes as a template argument the specific backend. The construction of the environment is the the programmer has to explicitely choose the backend he wants. The other parts of the library are completely independent of the backend from the viewpoint of the programmer, with the exception of _extensions_ which let backends provide additional functionality. More on this in [implementation]. The environment is responsible for:

- Initializing the parallel system.
- Obtaining information on high level properties such as the number of available processors.
- Describing how to spawn _kernels_ through the `spawn` function.

The spawn function creates a _world_ for each of the processors.

## World

Within the kernel, the world provides all the mechanisms that are necessary to communicate with other processors.

In particular, the world provides the following functionality:

- Registration of variables
- Communication using variables
- Message passing
- Processor information (id, active processors)
- Barrier synchronisations

Internally, a world also has a provider implementation _(world provider)_, which implements the transport layer and utility functions for a given backend. This world_provider can provide specialized extensions (e.g. hostname for MPI processors). It can be obtained using `world.provider()`.

## Glossary

- _SPMD_: (Single program multiple data) a parallel programming paradigm in which each of the processors executes the exact same code, but on different data.
- _Kernel_: the SPMD section that is executed by each [processor].
- _Processor_: this should be interpreted not as a physical processor, but as an abstract 'processing element', possibly virtual, that executes a [kernel].
