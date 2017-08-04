Coarrays
=========

Coarrays are a convenient way to store, and manipulate distributed
arrays. These *distributed arrays* can be seen as distributed variables,
whose images are a local array. With Bulk, we provide a coarray that is
modeled after [Coarray Fortran](https://en.wikipedia.org/wiki/Coarray_Fortran).

```cpp
auto xs = bulk::coarray<int>(world, 5);
```

Here, we create a coarray with local size equal to *5*. The total
number of elements in the coarray is therefore `5 * p`. We
use a constant size here, but this is not required as the local size is
allowed to vary over the processors. Coarrays provide syntactic sugar
to make manipulating distributed arrays as easy as possible. For
example, we can write:

```cpp
xs(3)[2] = 1;
```

This writes the value `1` to the element with local index
`2` on the processor with index `3`. The local
image of an array is iterable, so we can write for example:

```cpp
int result = 0;
for (auto x : xs) {
    result += x;
}
```

to compute the local sum of the numbers in the coarray image.

## Slices

Often, you need to deal with multiple elements of a coarray image at once. For
this, Bulk supports for slices. For example, to write to a range of elements at
once:

```cpp
auto xs = bulk::coarray<int>(world, 10);
xs(world.next_rank())[{2, 5}] = {2, 3, 4};
```

Or to get a range of elements:

```cpp
auto xs = bulk::coarray<int>(world, 10);
auto ys = xs(world.next_rank())[{2, 5}].get();
world.sync();
// ys[0], ys[1], ys[2] are now available;
```
