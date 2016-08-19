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

Next, we look at some basic forms of communication between processors. The main way to *talk* with other processors, is by using variables. A variable is created as follows:

.. code-block:: cpp

    auto x = bulk::create_var<T>(world);

Here, :code:`T` is the type of the variable, say an :code:`int`. Values can be assigned to the (local) variable:

.. code-block:: cpp

    x = 5;

The reason to use a variable, is that a processor can *write* to a remote **image** of a variable.

.. code-block:: cpp

    bulk::put(world.next_processor(), x, 4);

This will overwrite the value of the variable :code:`x` on the next logical processor (i.e. processor :code:`s + 1 % p`) with :code:`4`. We can obtain the value of a remote image using:

.. code-block:: cpp

    auto y = bulk::put(world.next_processor(), x);

Here, :code:`y` is a :code:`bulk::future` object.
