Bulk
====

Bulk is an alternative interface for writing parallel programs in C++ in bulk-synchronous style. The main goal of the project is to do away with the unnecessary boilerplate and ubiquitous pointer arithmetic that is find in for example the BSPlib standard. Furthermore, the interface supports and encourages the use of modern C++ features such as smart pointers, range based for loops and lambda functions.

Examples
--------


```cpp
// hello world
bulk::spawn(bulk::available_processors(), [](int s, int p) {
    std::cout << "Hello, world " << s << " / " << p << std::endl;
}

// communication
bulk::spawn(bulk::available_processors(), [](int s, int p) {
    bulk::var<int> a;

    bulk::put(bulk::next_processor(), s, a);
    bulk::sync();

    std::cout << s << " <- " << a.value << std::endl;

    auto b = bulk::get<int>(bulk::next_processor(), a);
    bulk::sync();

    std::cout << s << " -> " << b.value << std::endl;
});

// message passing
bulk::spawn(bulk::available_processors(), [](int s, int p) {
    for (int t = 0; t < p; ++t) {
        bulk::send<int, int>(t, s, s);
    }

    bulk::sync();

    if (s == 0) {
        for (auto message : bulk::messages<int, int>()) {
            std::cout << message.tag << ", " << message.content << std::endl;
        }
    }
});
```

Installing
----------

Authors
-------

* Jan-Willem Buurlage
* Tom Bannink

License
----------

MIT

Contributing
------------

Issues, PRs, Documentation
