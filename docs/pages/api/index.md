Bulk API reference
==================

|                                          |                                                                 |
|------------------------------------------|-----------------------------------------------------------------|
| **System**                               |                                                                 |
| [`bulk::hub`](hub.md)                    | the central object that encapsulates the distributed system     |
| [`bulk::provider`](provider.md)          | an implementation of the low-level functions                    |
| **Distributed objects**                  |                                                                 |
| [`bulk::var`](hub.md)                    | a distributed variable with an image for each processor         |
| [`bulk::future`](provider.md)            | an object which encapsulates a value known in future supersteps |
| [`bulk::array`](array.md)                | a distributed array with a local array image for each processor |
| [`bulk::coarray`](coarray.md)            | a co-array implementation on top of `array`                     |
| **Message passing**                      |                                                                 |
| [`bulk::message`](hub.md)                | an object containing a message                                  |
| [`bulk::message_container`](provider.md) | a container containing messages                                 |
| **Algorithm**                            |                                                                 |
| [`bulk::reduce`](hub.md)                 | a left fold over a `var`                                        |
| **Utility**                              |                                                                 |
| [`bulk::log`](hub.md)                    | race-free logging                                               |


