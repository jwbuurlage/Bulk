.. sectionauthor:: Jan-Willem Buurlage <janwillembuurlage@gmail.com>

.. highlight:: cpp

Main concepts
=============

The central object in a Bulk program is a `hub`. The hub provides information on the system, and a communication mechanism to the other processors. 

Initializing the system
-----------------------

To use Bulk, you can choose an implementation. The default implementation of a hub for shared memory systems is implemented with the C++ standard library. A simple C++ program written in Bulk looks like this::

    auto hub = bulk::bsp_hub();

    hub.spawn(hub.available_processors(), [&hub](int s, int p) {
        if (s == 0) {
            std::cout << "Hello, world " << s << "/" << p << std::endl;
        }
    });

Requesting system information
-----------------------------
