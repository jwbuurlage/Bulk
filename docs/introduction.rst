.. sectionauthor:: Jan-Willem Buurlage <janwillembuurlage@gmail.com>

.. highlight:: cpp

Introduction
============

Bulk is a library for writing parallel and distributed software. It consists of three layers:

* **Bulk Quark** is the lowest layer, and consists of low-level communication such as send and receive calls.
* **Bulk Atom** is the first level of abstraction, and is a higher level layer where objects such as coarrays are defined.
* **Bulk Molecule** is the final layer of abstraction, and supports operations commonly seen in application written in e.g. Hadoop or Spark, such as `map` or `reduce` operations.

Philosophy
----------

Bulk strives to provide a modern interface for writing performant parallel and distributed software. The lowest level is to be implemented with heavily optimized technologies such as MPI, while the higher levels provide a level of abstraction that allows average developers to develop highly efficient parallel software for both HPC and Big Data applications.

The Bulk project aims to provide the performance and fine-grain tunability of HPC software libraries such as MPI, with the ease-of-programming commonly found in Big Data platforms such as Hadoop and Spark.
