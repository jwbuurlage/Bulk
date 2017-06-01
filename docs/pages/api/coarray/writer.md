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
| `get`       | obtain a remote value              |


### `bulk::coarray::writer::operator=`

```cpp
void operator=(T value);
```

* **parameters** 
    * `value` - the new value of the element

### `bulk::coarray::writer::get`

```cpp
future<T> get();
```

* **returns** 
    * a future to the element
