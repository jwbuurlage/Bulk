Bulk
====

Bulk is an alternative interface for writing parallel programs in C++ in bulk-synchronous style. The main goal of the project is to do away with the unnecessary boilerplate and ubiquitous pointer arithmetic that is found in for example the BSPlib standard. Furthermore, the interface supports and encourages the use of modern C++ features such as smart pointers, range based for loops and lambda functions, enabling a new generation of programmers to write BSP programs.

Examples
--------

```cpp
auto env = bulk::environment<bulk::bsp::provider>();
env.spawn(env.available_processors(), [](auto world, int s, int p) {
    // 1. Hello world!
    BULK_IN_ORDER(
        std::cout << "Hello, world " << s << "/" << p << std::endl;
    )

    // 2. Communication
    auto a = world.create_var<int>();

    world.put(world.next_processor(), s, a);
    world.sync();

    // ... a.value() is now updated

    auto b = world.get<int>(world.next_processor(), a);
    world.sync();

    // ... b.value() is now available

    // 3. Message passing
    for (int t = 0; t < p; ++t) {
        world.send<int, int>(t, s, s);
    }

    world.sync();

    for (auto message : world.messages<int, int>()) {
        std::cout << message.tag << ", " << message.content << std::endl;
    }
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
