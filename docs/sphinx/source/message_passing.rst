Message passing
===============

Another way to communicate between processors is by using message queues. These queues can be used to send and receive an arbitrary number of *messages*. Messages have an attached *tag* and some *content*. Messages can be put into message queues, which have images on each processor. This queue can then be iterated over. To create a queue, you write:

.. code-block:: cpp

    auto queue = bulk::queue<int, float>(world);

This will create a queue that stores message with *integer* tags, and *float* content. For example, a message can correspond to a component of a vector of floats. To put a message into a remote queue, we use :code:`queue(pid).send`:

.. code-block:: cpp

    queue(world.next_processor()).send(1, 1.0f);
    queue(world.next_processor()).send(2, 5.0f);

This will send two messages to the next logical processor, with tags `1` and `2` respectively, and with contents `1.0f` and `5.0f`. As with communication through variables, this mechanism is also *bulk-synchronous*, which means that the remote queue will only have access to the messages in the next superstep.

.. warning::
    Message queues, like variables, are identified by the order in which they are constructed. Make sure this order is the same on each processor.

.. code-block:: cpp

    world.sync();

    for (auto msg : queue) {
        std::cout << "Received tag: " << msg.tag << " with content " << msg.content << "\n";
    };

It is perfectly legal, and even encouraged, to make a seperate queue for different types of messages. Each message queue has its own independent tag type and content type. For example:

.. code-block:: cpp

    auto q = bulk::queue<int, int>(world);
    q(world.next_processor()).send(1, 1);
    q(world.next_processor()).send(2, 3);
    q(world.next_processor()).send(123, 1337);

    auto q2 = bulk::queue<int, float>(world);
    q2(world.next_processor()).send(5, 2.1f);
    q2(world.next_processor()).send(3, 4.0f);

    world.sync();

    // read queue
    for (auto& msg : q) {
        std::cout << "the first queue received a message:" << msg.tag << ", "
                  << msg.content << "\n";
    }

    for (auto& msg : q2) {
        std::cout << "the second queue received a message:" << msg.tag << ", "
                  << msg.content << "\n";
    }
