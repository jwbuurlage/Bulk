# `bulk::coarray::slice_writer`

Defined in header `<bulk/coarray.hpp>`.

```cpp
class slice_writer;
```

Modify remote slices.

## Member functions
|             |                                    |
|-------------|------------------------------------|
| `operator=` | assign a value to a remote element |
| `get`       | get a future to a slice            |


### `bulk::coarray::slice_writer::operator=`

```cpp
void operator=(const std::vector<T>& values);
```

* **parameters** 
    * `values` - the new values of the slice

### `bulk::coarray::slice_writer::get`

```cpp
future<T[]> get();
```

* **returns** 
    * a future array containing the slice
