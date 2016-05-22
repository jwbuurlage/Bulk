# `bulk::coarray`

Defined in header `<bulk/coarray.hpp>`.

```cpp
template <typename T, class World>
class coarray;
```

Distributed array with easy element access, loosely based on the behaviour of [Co-Array Fortran](https://en.wikipedia.org/wiki/Coarray_Fortran).

## Usage

Co-arrays provide a convenient way to share data across processors. Instead of
manually sending and receiving data elements, co-arrays model distributed data
as a 2-dimensional array, where the first dimension is over the processors,
and the second dimension is over local 1-dimensional array indices.

```cpp
auto xs = create_coarray<int>(world, 10);
// set the 5th element on the 1st processor to 4
xs(1)[5] = 4;
// set the 3rd element on the local processor to 2
xs[3] = 2;
```

## Template parameters

* `T` - the type of the value stored in the local image of the coarray.
* `World` - the type of world to which this coarray belongs.

## Member classes
|                               |                                                  |
|-------------------------------|--------------------------------------------------|
| [`image`](coarray/image.md)   | representation of coarray image                  |
| [`writer`](coarray/writer.md) | allows for modification for remote coarray image |

## Member functions
|                                                     |                                                |
|-----------------------------------------------------|------------------------------------------------|
| [(constructor)](coarray/constructor.md)             | constructs the coarray                         |
| [(deconstructor)](coarray/deconstructor.md)         | deconstructs the coarray                       |
| **Value access**                                    |                                                |
| [`operator()`](coarray/parentheses_operator.md)     | obtain an image of the coarray                 |
| [`operator[]`](coarray/square_brackets_operator.md) | access the local elements of the coarray       |
| **World access**                                    |                                                |
| [`world`](coarray/world.md)                         | returns the world to which the coarray belongs |

## See also

- [`create_coarray`](coarray/create_coarray.md)