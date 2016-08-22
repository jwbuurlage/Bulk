.. Bulk documentation master file, created by
   sphinx-quickstart on Tue May 17 21:40:28 2016.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Welcome to |project_name|'s documentation!
================================

|project_name| is a common interface for writing parallel software. It makes it easy to write portable parallel code for a variety of platforms. It can be viewed as a spritual successor to BSPlib_.
Our main goal is to have a unified interface for programming modern computers, which are increasingly parallel and heterogeneous, while avoiding specialized code for specific platforms.

We accomplish this by providing backends for specific platforms. For example, we support native C++ threads, MPI and the Adapteva Epiphany platform, and plan to add additional backends in the future. The code written using Bulk follows the bulk-synchronous paradigm.

The |project_name| interface and reference backends are released under the MIT license. The source code and issue tracker are both hosted on GitHub_. The latest |project_name| release is |release|.

----

|project_name| is currently developed at CWI_ by `Jan-Willem Buurlage <mailto:j.buurlage@cwi.nl>`_ and `Tom Bannink <mailto:t.bannink@cwi.nl>`_.

----

.. toctree::
    :maxdepth: 2
    :caption: Introduction

    introduction
    getting_started
    tour

.. toctree::
    :maxdepth: 2
    :caption: User Documentation

    environment_world
    variables
    coarrays
    message_passing
    other_features

.. toctree::
    :maxdepth: 2
    :caption: Backends

    mpi
    epiphany

.. toctree::
    :maxdepth: 1
    :caption: Reference & Background

    bsp



Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`

.. _BSPlib: http://www.bsp-worldwide.org
.. _GitHub: http://www.github.com/jwbuurlage/Bulk
.. _CWI: http://www.cwi.nl
