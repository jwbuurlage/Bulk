Message passing
===============

Another way to communicate between processors is by using message
queues. These queues can be used to send and receive an arbitrary number
of *messages*. Messages have an attached *tag* and some *content*.
Messages can be put into message queues, which have images on each
processor. This queue can then be iterated over. To create a queue, you
write:

```cpp
auto queue = bulk::queue<int, float>(world);
```

This will create a queue that stores message with *integer* tags, and
*float* content. For example, a message can correspond to a component of
a vector of floats. To put a message into a remote queue, we use
`queue(pid).send`:

```cpp
queue(world.next_rank()).send(1, 1.0f);
queue(world.next_rank()).send(2, 5.0f);
```

This will send two messages to the next logical processor, with tags 1
and 2 respectively, and with contents 1.0f and 5.0f. As with
communication through variables, this mechanism is also
*bulk synchronous*, which means that the remote queue will only have
access to the messages in the next superstep.

!!! warning
    Message queues, like variables, are identified by the order in which
    they are constructed. Make sure this order is the same on each
    processor.

```cpp
world.sync();

for (auto [tag, content] : queue) {
    world.log("Received tag: %d and content %f", tag, content);
};
```

It is perfectly legal, and even encouraged, to make a separate queue for
different types of messages. Each message queue has its own independent
types. In addition, you are not limited to 'tag + content' type of messages, you can also send untagged data, or custom data such as index tuples, or even your own structs. For example:

```cpp
auto raw_queue = bulk::queue<int>(world);
raw_queue(world.next_rank()).send(1);
raw_queue(world.next_rank()).send(2);
raw_queue(world.next_rank()).send(123);

auto tuple_queue = bulk::queue<int, int, int>(world);
tuple_queue(world.next_rank()).send(1, 2, 3);
tuple_queue(world.next_rank()).send(4, 5, 6);

world.sync();

// read queue
for (auto x : raw_queue) {
    world.log("the first queue received a message: %d", x);
}

for (auto [i, j, k] : tuple_queue) {
    world.log("the second queue received a tuple: (%d, %d, %d)", i, j, k);
}
```

It is also possible to send arrays using queues, by having a message component
of type `T[]`.

Components of queues of type `T[]`, require a `std::vector<T>` as input to `send`.
Similarly, when iterating through the queue the `T[]` component of the message
will be represented by a `std::vector<T>`.

```cpp
auto q = bulk::queue<int[], int>(world);
q(world.next_rank()).send({1, 2, 3, 4}, 1);
world.sync();
for (auto [xs, y] : q) {
    // ... xs is of type std::vector<int>
}
```
