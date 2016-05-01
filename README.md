Bulk
====

Bulk is an alternative interface for writing parallel programs in C++ in bulk-synchronous style. The main goal of the project is to do away with the unnecessary boilerplate and ubiquitous pointer arithmetic that is found in for example the BSPlib standard. Furthermore, the interface supports and encourages the use of modern C++ features such as smart pointers, range based for loops and lambda functions.

Examples
--------

```cpp
// 1. Hello world!
auto center = bulk::center();
center.spawn(center.available_processors(), [&center](int s, int p) {
    std::cout << "Hello, world " << s << "/" << p << std::endl;
});

// 2. Communication
bulk::var<int> a;

center.put(center.next_processor(), s, a);
center.sync();

// ... a.value() is now updated

auto b = center.get<int>(center.next_processor(), a);
center.sync();

// ... b.value() is now available

// 3. Message passing
for (int t = 0; t < p; ++t) {
    center.send<int, int>(t, s, s);
}

center.sync();

for (auto message : center.messages<int, int>()) {
    std::cout << message.tag << ", " << message.content << std::endl;
}
```

Installing
----------

Authors
-------

* Jan-Willem Buurlage
* Tom Bannink

License
-------

MIT

Contributing
------------

Issues, PRs, Documentation
