# `bulk::future`

Defined in header `<bulk/future.hpp>`.

```cpp
template <typename T>
class future; // (1)

template <typename T>
class future<T[]>; // (2)
```

`bulk::future` represents a value (1) or array (2) that will or has become known in the superstep after its creation.

## Template parameters

* `T` - the type of the value(s) stored in the future

## Member functions

|                                              |                                             |
|----------------------------------------------|---------------------------------------------|
| [(constructor)](future/constructor.md)       | constructs the future                       |
| [(deconstructor)](future/deconstructor.md)   | deconstructs the future                     |
| [`operator=`](future/assignment_operator.md) | assign a future                             |
| **Value access**                             |                                             |
| [`value`](future/value.md)                   | returns the value of the future (1)         |
| [`operator[]`](future/bracket_operator.md)   | return an element of the array (2)          |
| **Hub access**                               |                                             |
| [`world`](future/world.md)                   | returns the hub to which the future belongs |
