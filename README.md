Bulk
====

Bulk is an alternative interface for writing parallel programs in C++ in bulk-synchronous style. The main goal of the project is to do away with the unnecessary boilerplate and ubiquitous pointer arithmetic that is found in for example the BSPlib standard. Furthermore, the interface supports and encourages the use of modern C++ features such as smart pointers, range based for loops and lambda functions, enabling a new generation of programmers to write BSP programs.

Examples
--------

```cpp
auto env = bulk::environment<bulk::bsp::provider>();
env.spawn(env.available_processors(), [](auto world, int s, int p) {
    // 1. Hello world!
    world.log("Hello, world %d/%d\n", s, p);

    // 2. Communication
    auto a = world.create_var<int>();

    a(world.next_processor()) = s;
    world.sync();

    // ... the local a is now updated

    auto b = a(world.next_processor()).get();
    world.sync();

    // ... b.value() is now available

    // 3. Message passing
    auto q = bulk::create_queue<int, int>(world);
    for (int t = 0; t < p; ++t)
        q(t).send(s, s); // send (s,s) to processor t

    world.sync();

    // Messages are now available in q
    for (auto& msg : q)
        world.log("%d got sent %d, %d\n", s, msg.tag, msg.content);
});

```

Authors
-------

Bulk is developed by:

* Jan-Willem Buurlage (jwbuurlage)
* Tom Bannink (tombana)

License
-------

Bulk is released under the MIT license, for details see the file LICENSE.md.

Contributing
------------

We welcome contributions. Please submit pull requests against the develop branch.
