# Partitionings

## `bulk::index_type`

Defined in header `<bulk/partitionings/partitioning.hpp>`.

```cpp
template <int D>
using index_type = std::array<int, D>;
```

## `bulk::partitioning`

Defined in header `<bulk/partitionings/partitioning.hpp>`.

```cpp
template <int D>
class partitioning;
```

Base class for partitionings over a 1D processor grid.

### Template parameters

- `D` - the dimension of the data space

### Member functions

|                                                  |                                       |
|--------------------------------------------------|---------------------------------------|
| [(constructor)](partitioning/constructor.md)     | constructs the partitioning           |
| [(deconstructor)](partitioning/deconstructor.md) | deconstructs the partitioning         |
| [`global_size`](partitioning/global_size.md)     | returns the global data shape         |
| [`local_size`](partitioning/local_size.md)       | returns the local data shape          |
| [`local_count`](partitioning/local_count.md)     | returns the number of local elements  |
| [`owner`](partitioning/owner.md)                 | returns the owner of a global index   |
| [`local`](partitioning/local.md)                 | convert a global index to a local one |
| [`global`](partitioning/global.md)               | convert a local index to a global one |

## `bulk::multi_partitioning`

Defined in header `<bulk/partitionings/partitioning.hpp>`.

```cpp
template <int D, int G>
class multi_partitioning : public partitioning<D>;
```

Base class for partitionings over a processor grid of dimension `G`.

### Template parameters

- `D` - the dimension of the data space
- `G` - the dimension of the processor grid

### Member functions

|                                                        |                                              |
|--------------------------------------------------------|----------------------------------------------|
| [(constructor)](multi_partitioning/constructor.md)     | constructs the partitioning                  |
| [(deconstructor)](multi_partitioning/deconstructor.md) | deconstructs the partitioning                |
| [`local_size`](multi_partitioning/local_size.md)       | returns the local data shape                 |
| [`global`](multi_partitioning/global.md)               | convert a local index to a global one        |
| [`multi_rank`](multi_partitioning/multi_rank.md)       | convert a 1D processor rank to its grid rank |
| [`grid`](multi_partitioning/grid.md)                   | returns the processor grid shape             |
| [`rank`](multi_partitioning/rank.md)                   | convert a grid rank to a 1D processor rank   |
| [`multi_owner`](multi_partitioning/multi_owner.md)     | get the multi rank of a global index         |


## `bulk::rectangular_partitioning`

Defined in header `<bulk/partitionings/partitioning.hpp>`.

```cpp
template <int D, int G>
class rectangular_partitioning : public multi_partitioning<D, G>;
```

Base class for partitionings over a processor grid of dimension `G`, where the
local data of each processor is a (hyper)rectangular subregion of the global
data space.

### Template parameters

- `D` - the dimension of the data space
- `G` - the dimension of the processor grid

### Member functions

|                                                        |                                           |
|--------------------------------------------------------|-------------------------------------------|
| [(constructor)](rectangular_partitioning/constructor.md) | constructs the partitioning               |
| [(deconstructor)](rectangular_partitioning/deconstructor.md) | deconstructs the partitioning             |
| [`origin`](rectangular_partitioning/origin.md)    | returns the origin of the local subregion |

## `bulk::cartesian_partitioning`

Defined in header `<bulk/partitionings/partitioning.hpp>`.

```cpp
template <int D, int G = D>
class cartesian_partitioning : public multi_partitioning<D, G>;
```

Base class for partitionings over a processor grid of dimension `G`, where the
each axis is partitioned independently.

### Template parameters

- `D` - the dimension of the data space
- `G` - the dimension of the processor grid

### Member functions

|                                                        |                                       |
|--------------------------------------------------------|---------------------------------------|
| [(constructor)](cartesian_partitioning/constructor.md) | constructs the partitioning           |
| [(deconstructor)](cartesian_partitioning/deconstructor.md) | deconstructs the partitioning         |
| [`owner`](cartesian_partitioning/owner.md)             | returns the owner of a global index   |
| [`local`](cartesian_partitioning/local.md)             | convert a global index to a local one |
| [`global`](cartesian_partitioning/global.md)           | convert a local index to a global one |
| [`local_size`](cartesian_partitioning/local_size.md)   | returns the local data shape          |

## `bulk::cyclic_partitioning`

Defined in header `<bulk/partitionings/cyclic.hpp>`.

```cpp
template <int D, int G = D>
class cyclic_partitioning : public cartesian_partitioning<D, G>;
```

A cyclic partitioning distributes the indices of a D-dimension space over the
first G axes, where G is the dimensionality of the processor grid.

## `bulk::block_partitioning`

Defined in header `<bulk/partitionings/block.hpp>`.

```cpp
template <int D, int G = D>
class block_partitioning : public rectangular_partitioning<D, G>;
```

A block distribution. This equally block-distributes the first G axes.
Optionally, a third argument `axes`, an array of size `G` that indicates the
axes over which to partition, can be supplied to the constructor.

## `bulk::tree_partitioning`

Defined in header `<bulk/partitionings/tree.hpp>`.

```cpp
template <int D>
class tree_partitioning : public rectangular_partitioning<D, 1>
```

A binary-space partitioning (abbreviated BSP).

```
             (a_0, d_0)
             /        \
      (a_10, d_10) (a_11, d_11)
      /      \       /     \
     ...     ...    ...    ...
      |       |      |      |
(a_n0, d_n0) ...    ...  (a_n(p/2), d_n(p/2))
```

represented as binary tree of splits.

### (constructor)

```cpp
tree_partitioning(index_type<D> data_size, int procs,
                  util::binary_tree<util::split>&& splits)
```

#### Parameters

* `data_size` - the shape of the data
* `procs` - the number of processors to distribute over
* `splits` - a binary tree, each node has associated a pair `(a, d)` with the
  location `a`, and axis `d`, among which is recursively split.
