.. code-highlight:: cpp

A tour of |project_name|
========================

On this page we aim to highlight some of the main features of the |project_name| library, and to give an impression of the way code written with |project_name| looks.

Hello :code:`bulk::world`!
--------------------------

We start out with the obligatory Hello World! in Bulk |project_name|, and subsequently explain the code line-by-line. In this code we will use the *MPI* backend, but everything written here is completely general, and guarenteed to work on top of any conforming Bulk backend.

.. code-block:: cpp
    :linenos:

    #include <bulk/bulk.hpp>
    #include <bulk/backends/mpi/mpi.hpp>

    int main() {
        auto env = bulk::environment<bulk::mpi::provider>();
        env.spawn(env.available_processors(), [](auto world, int s, int p) {
            std::cout << "Hello, world " << s << "/" << p << std::endl;
        }
    }

On lines `1` and `2` we include the |project_name| library, and the backend of our choosing (in our case MPI). On line `5`, we initialize an **environment**, which sets up the parallel or distributed system.

On line `6`, we spawn the **SPMD** section of our program within the environment. The first argument denotes the number of processors that we want to run the section on, while the second argument provides a function-like object (here a C++ lambda function) that is executed on the requested number of processors. This function obtains a **World** object, which it can use to communicate with other processors, for programmer convenience its identifier :code:`s` and the total number of processes that are spawned :code:`p` are passed as well. These can alternatively be obtained from world using :code:`world.processor_id()` and :code:`world.active_processors()` respectively.

Communication between processors
--------------------------------

Next, we look at some basic forms of communication between processors. The main way to *talk* to other processors, is by using variables. A variable is created as follows:

.. code-block:: cpp

    auto x = bulk::create_var<T>(world);

Here, :code:`T` is the type of the variable, for example an :code:`int`. Values can be assigned to the (local) variable:

.. code-block:: cpp

    x = 5;

The reason to use a variable, is that a processor can *write* to a remote **image** of a variable.

.. code-block:: cpp

    bulk::put(world.next_processor(), 4, x);

This will overwrite the value of the variable :code:`x` on the next logical processor (i.e. processor :code:`s + 1 % p`) with :code:`4`. We can obtain the value of a remote image using:

.. code-block:: cpp

    auto y = bulk::get(world.next_processor(), x);

Here, :code:`y` is a :code:`bulk::future` object. A future object does not immediately hold the remote value of :code:`x`, but after a *future* call to :code:`world.sync()`, we can extract the remote value out of :code:`y`.

.. code-block:: cpp

    world.sync();
    auto x_next = y.value();

Co-arrays
---------

Co-arrays are a convenient way to store, and manipulate distributed data. We provide a co-array that is modeled after `Co-array Fortran`_. Arrays are initialized and used as follows:

.. code-block:: cpp

    auto xs = bulk::create_coarray<int>(world, s);
    xs(3)[2] = 1;

Here, we create a co-array of varying local  size (each processor holds :code:`s` many elements). Next we write the value :code:`1` to the element with local index :code:`2` on processor with index :code:`3`.

Algorithmic skeletons
---------------------

|project_name| comes equipped with a number of higher-level functions, also known as *algorithmic skeletons*. For example, say we want to compute the dot-product of two coarrays, then we write this as:

.. code-block:: cpp

    auto xs = bulk::create_coarray<int>(world, s);
    auto ys = bulk::create_coarray<int>(world, s);

    // fill xs and ys with data
    auto result = bulk::create_var<int>(world);
    for (int i = 0; i < s; ++i) {
        result.value() += xs[i] * ys[i];
    }

    // reduce to find global dot product
    auto alpha = bulk::foldl(result, [](int& lhs, int rhs) { lhs += rhs; });

Here we first compute the local inner product, and finally use the higher-level function :code:`bulk::foldl` to find the global result.

Another example is finding a maximum element over all processors, here max is the maximum value found locally:

.. code-block:: cpp

    auto maxs = bulk::gather_all(world, max);
    max = *std::max_element(maxs.begin(), maxs.end());

Compare this to the way this is done using e.g. BSPlib:

.. code-block:: cpp

    int* global_max = malloc(sizeof(int) * bsp_nprocs());
    bsp_push_reg(global_max, sizeof(int) * bsp_nprocs());

    for (int t = 0; t < p; ++t) {
        bsp_put(t, &max, global_max, bsp_pid(), sizeof(int));
    }
    bsp_sync();

    for (int t = 0; t < p; ++t) {
        if (max < global_max[t]) {
            max = global_max[t];
        }
    }

.. _Co-array Fortran: https://en.wikipedia.org/wiki/Coarray_Fortran
