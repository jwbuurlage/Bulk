# `bulk::future::operator=`

```cpp
void operator=(future<T>&& other);
```

Move assignment operator. Replaces the target future `*this` with the source future `other`, and invalidates the source.

## Parameters

* `other` - another future to move away from
