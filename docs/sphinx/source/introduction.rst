Introduction
============

About |project_name|
--------------------

|project_name| is a C++ library for writing parallel and distributed software. The main motivation is to provide a modern interface for writing parallel scientific and HPC software, but the resulting interface is general enough to use for many other applications.

It provides a modern interface for writing performant parallel and distributed software. Internally, the specific code for communicating (the transport layer) is implemented by backends based on heavily optimized technologies such as MPI, while the higher levels provide a level of abstraction that allows average developers to develop efficient parallel software.

The |project_name| project aims to provide the performance and fine-grain tunability and control of classic HPC software libraries such as MPI, with the ease-of-programming commonly found in Big Data platforms such as Hadoop and Spark.

Why |project_name|
--------

.. image:: https://imgs.xkcd.com/comics/standards.png
    :align: center

Structure of this documentation
-------------------------------

Bulk provides a modern bulk-synchronous parallel API which is `documented in detail here <api/index.html>`_. In this documentation we will first give a tour of Bulk, and subsequently a detailed introduction is given to each component of |project_name|.

Related projects
----------------

This is an incomplete list of alternative libraries for parallel computing.

General parallel libraries:

- MPI
- OpenMP

BSP specific:

- BSPonMPI
- Multicore-BSP
- Apache Hama

Libraries based on algorithmic skeletons:

- Muesli - the Muenster skeleton library
- SkePU 2

GPU libraries:

- OpenCL
- CUDA
- Vulkan (compute)
- Thrust
- SYCL

Big Data:

- Apache Hadoop
- Apache Spark
- Apache Giraph
- TensorFlow

A number of alternative libraries for Parallella exist:

- Epiphany BSP
- COPRTHR-2 with MPI support
- Epiphany Erlang
