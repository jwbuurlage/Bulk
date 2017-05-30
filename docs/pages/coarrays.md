Coarrays
=========

Coarrays are a convenient way to store, and manipulate distributed
arrays. These *distributed arrays* can be seen as distributed variables,
whose images are a local array. With Bulk, we provide a co-array that is
modeled after [Coarray Fortran](https://en.wikipedia.org/wiki/Coarray_Fortran).

```cpp
auto xs = bulk::coarray<int>(world, 5);
```

Here, we create a co-array with local size equal to *5*. The total
number of elements in the co-array is therefore `5 * p`. We
use a constant size here, but this is not required as the local size is
allowed to vary over the processors. Coarrays provide syntactic sugar
to make manipulating distributed arrays as easy as possible. For
example, we can write:

```cpp
xs(3)[2] = 1;
```

This writes the value `1` to the element with local index
`2` on processor with index `3`. The local
image of an array is iterable, so we can write for example:

```cpp
int result = 0;
for (auto x : xs) {
    result += x;
}
```

To compute the local sum of the numbers in the co-array image.
