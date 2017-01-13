Introduction
============

About |project_name|
--------------------

|project_name| is a C++ library for writing parallel and distributed software. The main motivation is to provide a modern interface for writing parallel scientific and HPC software, but the resulting interface is general enough to use for many other applications.

It provides a modern interface for writing performant parallel and distributed software. Internally, the specific code for communicating (the transport layer) is implemented by backends based on heavily optimized technologies such as MPI, while the higher levels provide a level of abstraction that allows average developers to develop efficient parallel software.

The |project_name| project aims to provide the performance and fine-grain tunability and control of classic HPC software libraries such as MPI, with the ease-of-programming commonly found in Big Data platforms such as Hadoop and Spark.

Why |project_name|
--------

|project_name| is a general purpose library for distributed computing, which does not put any restrinctions on the programs that can be expressed using the library. At the same time, it enables the use of common programming abstractions, by supporting and encouraging the use of modern C++ idioms, such as the use of anonymous functions, smart pointers, range based for loops and portable threading support.

Structure of this documentation
-------------------------------

|project_name| provides a modern bulk-synchronous parallel API which is `documented in detail here <api/index.html>`_. In this documentation we will first give a tour of Bulk, and subsequently a detailed introduction is given to each component of |project_name|.
