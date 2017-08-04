Bulk
====

<center>

![image](images/logo.svg)

</center>

Bulk is a new interface for writing parallel programs in C++. The library does away with the unnecessary boilerplate and ubiquitous pointer arithmetic that is found in libraries based on for example MPI, or the BSPlib standard. Our BSP interface supports and encourages the use of modern C++ features such as smart pointers, range-based for-loops, anonymous functions, and structured bindings enabling safer and more efficient distributed programming. The flexible backend architecture ensures the portability of parallel programs written with Bulk.

About BSP
---------

The bulk synchronous parallel (BSP) programming model provides a way of writing parallel and distributed programs. BSP is the underlying model for Bulk. Instead of communicating between processors (or nodes, or cores) asynchronously, all communication is staged and resolved at fixed _synchronization points_. These synchronizations delimit so-called _supersteps_. This way of writing parallel programs has a number of advantages over competing models, at the cost of losing some flexibility and possible performance in terms of communication.

- The resulting programs are structured, easy to understand and maintain, and can be reasoned about
- Data races are eliminated almost by construction, only having to follow a small set of simple rules.
- Scalability is easy to obtain, since programs are written in a SPMD[^1] fashion, and the cost of a program can be analyzed explicitely.
- The number of communication mechanisms required are very limited, roughly only distinguishing between anonymous or named communication. This makes BSP based libraries very economic (you can do much with very little).
- It has a low cost of entry. It is easy to write _correct_ BSP programs, while it is notoriously hard to write correct asynchronous parallel programs.


Authors
-------

Bulk is developed by:

* Jan-Willem Buurlage (jwbuurlage)
* Tom Bannink (tombana)

Examples
--------

Hello world!

```cpp
bulk::thread::environment env;
env.spawn(env.available_processors(), [](auto& world) {
    auto s = world.rank();
    auto p = world.active_processors();

    world.log("Hello world from processor %d / %d\n", s, p);
});
```

Distributed variables are the easiest way to communicate.

```cpp
auto a = bulk::var<int>(world);
a(world.next_rank()) = s;
world.sync();
// ... a is now updated

auto b = a(world.next_rank()).get();
world.sync();
// ... b.value() is now available
```

Coarrays provide a convenient syntax for working with distributed arrays.

```cpp
auto xs = bulk::coarray<int>(world, 10);
xs(world.next_rank())[3] = s;
```

Message passing can be used for more flexible communication.

```cpp
auto q = bulk::queue<int, float>(world);
for (int t = 0; t < p; ++t) {
    q(t).send(s, 3.1415f);  // send (s, pi) to processor t
}
world.sync();

// messages are now available in q
for (auto [tag, content] : q) {
    world.log("%d got sent %d, %f\n", s, tag, content);
}
```

API
---

Bulk provides a modern bulk synchronous parallel API which is [documented in detail here](api/index.md).

License
-------

Bulk is released under the MIT license, for details see the file LICENSE.md.

Contributing
------------

We welcome contributions. Please submit pull requests against the develop branch.


[^1]: Single program multiple data, meaning that every processor runs the same code but on different data.
