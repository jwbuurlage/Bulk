Getting started
===============

Currently, there are no binary releases for Bulk. To start using Bulk in its current state, we suggest you clone the repository:

.. code-block:: bash

    git clone https://github.com/jwbuurlage/bulk/

Afterwards, you must choose a backend, and install the necessary dependencies for that backend (if any). For example, to build the MPI examples:

.. code-block:: bash

    cd bulk/backends/mpi/build
    cmake .
    make

The example binaries can be found in the `../bin` folder.
