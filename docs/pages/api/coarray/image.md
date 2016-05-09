# `bulk::coarray::image`

Defined in header `<bulk/coarray.hpp>`.

```cpp
class image;
```

A coarray image, allows for remote element access

## Member functions
|               |                                            |
|---------------|--------------------------------------------|
| `operator[]`  | returns a writer to a remote image element |


### `bulk::coarray::image::operator[]`

```cpp
writer operator[](int i);
```

* **parameters** 
    * `i` - the index of the image element
* **returns** a `bulk::coarray::writer` to the remote image element
