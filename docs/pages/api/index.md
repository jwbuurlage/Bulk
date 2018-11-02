Bulk API reference
==================

|                                                |                                                                 |
|------------------------------------------------|-----------------------------------------------------------------|
| **System**                                     |                                                                 |
| [`bulk::environment`](environment.md)          | the central object that encapsulates the distributed system     |
| [`bulk::world`](world.md)                      | an implementation of the low-level functions                    |
| **Distributed objects and communication**      |                                                                 |
| [`bulk::var`](var.md)                          | a distributed variable with an image for each processor         |
| [`bulk::future`](future.md)                    | an object which encapsulates a value known in future supersteps |
| [`bulk::coarray`](coarray.md)                  | a distributed array with a local array image for each processor |
| **Message passing**                            |                                                                 |
| [`bulk::queue`](queue.md)                 	 | a container containing messages                                 |
| **Algorithms**                                 |                                                                 |
| [`bulk::foldl`](foldl.md)                      | a left fold over a `var`                                        |
| [`bulk::gather_all`](gather_all.md)            | gather results                                                  |
| **Partitionings**                              |                                                                 |
| [`bulk::partitioning`](partitioning.md)        | index computations for data distributions |
| **Utility**                                    |                                                                 |
| [`bulk::util::timer`](timer.md)                | wall timer for benchmarking                                     |
| [`bulk::util::flatten`](flatten.md)            | flatten multi-indices                                           |
| [`bulk::util::unflatten`](unflatten.md)        | unflatten mutli-indices                                         |
