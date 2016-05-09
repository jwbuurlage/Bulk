Bulk
====

Bulk is an alternative interface for writing parallel programs in C++ in bulk-synchronous style. The main goal of the project is to do away with the unnecessary boilerplate and ubiquitous pointer arithmetic that is found in for example the BSPlib standard. Furthermore, the interface supports and encourages the use of modern C++ features such as smart pointers, range based for loops and lambda functions.

Examples
--------

```cpp
auto hub = bulk::bsp_hub();
hub.spawn(hub.available_processors(), [&hub](int s, int p) {
    // 1. Hello world!
    BULK_IN_ORDER(
        std::cout << "Hello, world " << s << "/" << p << std::endl;
    )

    // 2. Communication
    auto a = hub.create_var<int>();

    hub.put(hub.next_processor(), s, a);
    hub.sync();

    // ... a.value() is now updated

    auto b = hub.get<int>(hub.next_processor(), a);
    hub.sync();

    // ... b.value() is now available

    // 3. Message passing
    for (int t = 0; t < p; ++t) {
        hub.send<int, int>(t, s, s);
    }

    hub.sync();

    for (auto message : hub.messages<int, int>()) {
        std::cout << message.tag << ", " << message.content << std::endl;
    }
});

```

Installing
----------

Authors
-------

* Jan-Willem Buurlage

License
-------

MIT

Contributing
------------

Issues, PRs, Documentation
