Bulk API reference
==================

|                                                   |                                                                 |
|---------------------------------------------------|-----------------------------------------------------------------|
| **System**                                        |                                                                 |
| [`bulk::hub`](hub.md)                             | the central object that encapsulates the distributed system     |
| [`bulk::provider`](provider.md)                   | an implementation of the low-level functions                    |
| **Distributed objects and communication**         |                                                                 |
| [`bulk::var`](var.md)                             | a distributed variable with an image for each processor         |
| [`bulk::future`](future.md)                       | an object which encapsulates a value known in future supersteps |
| [`bulk::array`](array.md)                         | a distributed array with a local array image for each processor |
| [`bulk::coarray`](coarray.md)                     | a co-array implementation on top of `array`                     |
| [`bulk::put`](put.md)                             | send information to a remote processor                          |
| [`bulk::get`](get.md)                             | obtain information from a remote processor                      |
| **Message passing**                               |                                                                 |
| [`bulk::message`](message.md)                     | an object containing a message                                  |
| [`bulk::message_container`](message_container.md) | a container containing messages                                 |
| **Algorithm**                                     |                                                                 |
| [`bulk::reduce`](reduce.md)                       | a left fold over a `var`                                        |
| **Utility**                                       |                                                                 |
| [`bulk::log`](log.md)                             | race-free logging                                               |


