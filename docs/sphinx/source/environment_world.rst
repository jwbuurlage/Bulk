Environment and worlds
======================

Parallel environments
---------------------

A |project_name| program runs in some parallel environment. For example, this environment could be an MPI cluster, a many-core co-processor, or simply threads on a multi-core computer. This environment is accessed within the program through a `bulk::environment <api/environment.html>`_ object. This object is specialized for each *backend*, which is an implementation of the lower-level communication that reflect the actual environment. For example, to setup a |project_name| environment on an MPI cluster, we would write:

.. code-block:: cpp

    #include <bulk/bulk.hpp>
    #include <bulk/backends/mpi/mpi.hpp>

    int main() {
        bulk::mpi::environment env;
    }

For a list of default providers, consult the *backends* section of this documentation.

This environment object contains information on the parallel system, for example we can request the number of processors that are available.

We note that throughout this documentation (and in the library), **processor** is a general term for the entity that executes the SPMD section (more on this later) and communicates with other processors -- this can be an MPI node, a core, or a thread, depending on the backend that is used, but they are all treated in the same manner.

.. code-block:: cpp

    auto processor_count = env.available_processors();

This information can be used to *spawn* the program on the right amount of processors. Programs written in Bulk follow that **SPMD** (Single Program Multiple Data) paradigm. This means that each processor executes the same code, but has its own (local) data that it manipulates. In Bulk, the SPMD section is a *function object*. This can be a C++ lambda, a :code:`std::function`, or a C function pointer. In this documentation we will use lambda functions for our examples.

This function will run on each processor, and should take three arguments. The first, contains the *world* object, which we will describe in detail in the next section. The final two arguments are respectively the local processor index, and the total number of processors that the SPMD section runs on. The SPMD section is executed in the following way:

.. code-block:: cpp

    env.spawn(env.available_processors(), [](auto& world, int s, int p) {
        std::cout << "Hello, world " << s << "/" << p << std::endl;
    }

The :code:`spawn` function takes two arguments. The first is the total number of processors to run the SPMD section on, here we simply use all the processors that are available. The second is the SPMD function itself, that is run on the set number of processors.

The world of a processor
------------------------

.. image:: images/environment.png
    :align: center

Each processor can communicate to other processors using the *world* object of type `bulk::world <api/world.html>`_. The world object contains some information on the specifics of SPMD section, such as the number of processors executing the section, and its identifier (as we have seen, these are also provided as arguments for programmer convenience). We can also obtain indices of the neighbouring processors:

.. code-block:: cpp

    auto t = world.processor_id();
    auto q = world.active_processors();
    auto next = world.next_processor();
    auto previous = world.prev_processor();

Using the naming convention seen before, we would have :code:`t == s` and :code:`q == p`. The next and previous processor can also be computed very simply manually using:

.. code-block:: cpp

    next = (s + 1) % p;
    previous = (s + p - 1) % p;

However, we would suggest using the appropriate methods of world to increase readability. Another important mechanism exposed through the world object is the ability to perform a *bulk-synchronization*, which is the cornerstone of programs written in BSP style:

.. code-block:: cpp

    world.sync();

We will see the specific uses of bulk-synchronization in the upcoming sections.

Multiple environments
---------------------

It is possible to nest environments. For example, each MPI node could have a multi-core processor and could in addition have a many-core co-processor. In this case, using |project_name| provides a large benefit: it provides a unified syntax for each *layer*. We conclude this section with a complete *Hello world!* program that runs on an MPI cluster of Parallella's:

.. code-block:: cpp
    :linenos:

    #include <bulk/bulk.hpp>
    #include <bulk/backends/mpi/mpi.hpp>
    #include <bulk/backends/epiphany/epiphany.hpp>

    int main() {
        // initialize the outer MPI layer
        bulk::mpi::environment mpi_env;

        mpi_env.spawn(mpi_env.available_processors(), [](auto& mpi_world, int t, int q) {
            // initialize the inner Epiphany layer
            bulk::epiphany::environment epi_env;

            // on each MPI node, we run a parallel program on the Epiphany co-processor
            epi_env.spawn(epi_env.available_processors(), [](auto& world, int s, int p) {
                std::cout << "Hello, world " << s << "/" << p << std::endl;
            }
        }
    }
