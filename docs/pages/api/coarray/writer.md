# `bulk::coarray::writer`

Defined in header `<bulk/coarray.hpp>`.

```cpp
class writer;
```

A coarray image writer, allows for the modification of remote elements.

## Member functions
|             |                                    |
|-------------|------------------------------------|
| `operator=` | assign a value to a remote element |


### `bulk::coarray::writer::operator=`

```cpp
void operator=(T value);
```

* **parameters** 
    * `value` - the new value of the element
