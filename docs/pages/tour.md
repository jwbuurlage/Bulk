A tour of Bulk
==============

On this page we aim to highlight some of the main features of the
library, and to give an impression of the overall syntax.

Hello `bulk::world`!
---------------------------------

We start out with the obligatory Hello World! in Bulk , and subsequently
explain the code line-by-line. In this code we will use the *MPI*
backend, but everything written here is completely general, and
guaranteed to work on top of any conforming Bulk backend.

```cpp
#include <bulk/bulk.hpp>
#include <bulk/backends/mpi/mpi.hpp>

int main() {
    bulk::mpi::environment env;
    env.spawn(env.available_processors(), [](auto& world) {
        auto s = world.rank();
        auto p = world.active_processors();

        world.log("Hello world from processor %d / %d!", s, p);
    });
}
```

On lines 1 and 2 we include the library, and the backend of our choosing
(in our case MPI). On line 5, we initialize an **environment**, which
sets up the parallel or distributed system.

On line 6, we spawn the **SPMD** section of our program within the
environment. The first argument denotes the number of processors that we
want to run the section on, while the second argument provides a
function-like object (here a C++ lambda function) that is executed on
the requested number of processors. This function obtains a
**bulk::world** object, which it can use to communicate with other
processors. For convenience we suggest to alias the processor identifier (or rank) to `s`
and the total number of processes that are spawned to `p`. As shown in the example, these can be obtained from world using
`world.rank()` and `world.active_processors()` respectively.

Communication between processors
--------------------------------

Next, we look at some basic forms of communication between processors.
The main way to *talk* to other processors, is by using variables. A
variable is created as follows:

```cpp
auto x = bulk::var<T>(world);
```

Here, `T` is the type of the variable, for example an
`int`. Values can be assigned to the (local) variable:

```cpp
x = 5;
```

The reason to use such a distributed variable, is that a processor can *write* to a
remote **image** of a variable.

```cpp
bulk::put(world.next_rank(), 4, x);
// or the short-hand:
x(world.next_rank()) = 4;
```

This will overwrite the value of the variable `x` on the
next logical processor (i.e. processor `(s + 1) % p`) with
`4`. We can obtain the value of a remote image using:

```cpp
auto y = bulk::get(world.next_rank(), x);
// or the short-hand:
auto y = x(world.next_rank()).get();
```

Here, `y` is a `bulk::future` object. A future
object does not immediately hold the remote value of `x`,
but after a *future* call to `world.sync()`, we can extract
the remote value out of `y`.

```cpp
world.sync();
auto x_next = y.value();
```

Coarrays
---------

Coarrays are a convenient way to store, and manipulate distributed
data. We provide a coarray that is modeled after [Coarray
Fortran](https://en.wikipedia.org/wiki/Coarray_Fortran). Arrays are
initialized and used as follows:

```cpp
auto xs = bulk::coarray<int>(world, s);
xs(3)[2] = 1;
```

Here, we create a coarray of varying local size (each processor holds
`s` many elements). Next we write the value
`1` to the element with local index `2` on
processor with index `3`.

Algorithmic skeletons
---------------------

Bulk comes equipped with a number of higher-level functions, also known as
*algorithmic skeletons*. For example, say we want to compute the
dot-product of two coarrays, then we write this as:

```cpp
auto xs = bulk::coarray<int>(world, s);
auto ys = bulk::coarray<int>(world, s);

// fill xs and ys with data
auto result = bulk::var<int>(world);
for (int i = 0; i < s; ++i) {
    result.value() += xs[i] * ys[i];
}

// reduce to find global dot product
auto alpha = bulk::foldl(result, [](int& lhs, int rhs) { lhs += rhs; });
```

Here we first compute the local inner product, and finally use the
higher-level function `bulk::foldl`
result.

Another example is finding a maximum element over all processors, here
max is the maximum value found locally:

```cpp
auto maxs = bulk::gather_all(world, max);
max = *std::max_element(maxs.begin(), maxs.end());
```
