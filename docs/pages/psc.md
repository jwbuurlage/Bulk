# Introduction<a id="sec-1"></a>

Large scale problems are common in scientific computing. For example, in the simulation of physical systems typically the grid size is chosen as fine as computational considerations allow for, and linear systems containing millions of equations and unknowns are solved routinely.

In this tutorial, we will discuss the implementation of a number of important parallel algorithms in scientific computing. The algorithms we consider are those explained in [Parallel Scientific Computing by Rob Bisseling](http://www.staff.science.uu.nl/~bisse101/). Implementations of these algorithms in BSPlib can be found in [BSPedupack](http://www.staff.science.uu.nl/~bisse101/Software/software.html).

Our discussion is stand-alone, however the focus is on the implementation of these algorithms in Bulk, and not the derivation or detailed description of the algorithms. The result of our efforts will be a library that contains support for important objects in parallel scientific computing:

1.  Distributed vector
2.  Distributed matrix (dense and sparse)

The algorithms we will implement are (in order):

1.  Computing the *inner product* of two vectors
2.  *Sorting* a vector
3.  Computing the *LU decomposition* of a matrix

Our intentions are educational, we will strive to design simple implementations of the algorithm and will not optimize for specific hardware, or e.g. exploit hybrid systems. Although we do not develop the algorithms for maximum performance, we will make sure that all our algorithms exhibit good scaling behavior.

In most source code examples, we assume that we have the following aliases:

```cpp
auto s = world.rank();
auto p = world.active_processors();
```

The full source code is available in `examples/psc` starting from Bulk version v1.2.0. Note that this code may be slightly different from what we discuss here, for clarity of presentation, or because it has been updated.

# the `psc::vector` class: the humble beginnings of our library<a id="sec-2"></a>

Before we are ready to describe and implement our first BSP algorithm, we will first define a distributed vector object. Since we will require multiple data distributions for the various algorithms we will implement, we will not assume any fixed one. The design of the vector object should be clear from the following example:

```cpp
env.spawn(env.available_processors(), [&](bulk::world& world) {
    auto s = world.rank();
    auto p = world.active_processors();

    // we choose n = 1e6
    auto size = 1000000;
    // as an example, we take a cyclic partitioning
    auto pi = bulk::cyclic_partitioning<1>({size}, {p});

    auto v = psc::vector<int>(world, pi);
    auto w = psc::vector<int>(world, pi);
    // ... fill with data

    // compute the inner product
    auto alpha = psc::dot(v, w);

    // we output the result
    world.log("%i", alpha);
});
```

A partitioning `pi` is associated to a distributed vector. In this case, the components of the vector are integers. A `psc::vector` is defined as follows.

```cpp
namespace psc {

template <typename T>
class vector {
  //...
```

Everything we define will be in a *parallel scientific computing* (`psc`) namespace. We make a generic implementation, that works for any `T`. A constructor for the vector class can look something like the following.

```cpp
private:
  bulk::world& world_;
  bulk::partitioning<1>& partitioning_;
  bulk::coarray<T> data_;

public:
  vector(bulk::world& world, bulk::partitioning<1>& partitioning)
      : world_(world), partitioning_(partitioning),
        data_(world_, partitioning.local_count(world_.rank()), (T)0) {}
// ...
```

A vector object has three member objects; a reference to the world, a reference to the partitioning, and a `coarray` object in which is stores the data. The local size of this coarray is the number of local elements that we have been assigned according to the partitioning; as obtained by `local_count`. With this constructor, all elements are default initialized to zero.

For completeness, we add some accessors and utility functions to our vector class:

```cpp
auto size() const { return data_.size(); }
auto global_size() const { return partitioning_.global_size(); }

bulk::world& world() const { return world_; }
bulk::partitioning<1>& partitioning() const { return partitioning_; }

auto begin() { return data_.begin(); }
auto end() { return data_.end(); }
```

This allows us to query the size of the vector, to obtain the communication world in which it is defined, the partitioning, and to use it as a generic container.

With the vector object in place, we are ready to define our first BSP algorithm; the inner product computation.

# `psc::dot`: the inner product of two vectors<a id="sec-3"></a>

Computing the inner product is fairly straightforward. We assume that the vectors are partitioned in the same manner. The algorithm proceeds in two steps. First, every processor computes the inner product of its local elements. Second, the local results are shared with an all-to-all broadcast and then accumulated resulting in the global result. There are multiple ways to achieve this using Bulk, but an elegent way is to use `bulk::foldl` over a `bulk::var`, which works as follows.

Consider a distributed variable `x`. We can interpret `x` as a virtual 1D array, where the index in the index into the array corresponds to a processor id.

| \(s\) | \(0\)  | \(1\)  | \(2\)  | &#x2026; | \(p - 1\)  |
|----- |------ |------ |------ |-------- |---------- |
| `x`   | `x(0)` | `x(1)` | `x(2)` | &#x2026; | `x(p - 1)` |

A *left fold* over a 1D array takes some binary operation `+`, and computes the reduction of the array using this operation:

```cpp
auto foldl = ((((x(0) + x(1)) + x(2)) + ...) + x(p - 1))
```

For the inner product, we actually want to use the sum as the binary operation, but if `T` would be for example a `bool`, we can also use an AND operation `&`, and a left fold would check if all local values are set to `true`.

With the definition of a left fold in place, we are ready to describe the inner product algorithm.

```cpp
template <typename T>
T dot(const vector<T>& x, const vector<T>& y) {
    bulk::var<T> result(x.world());

    for (auto i = 0u; i < x.size(); ++i) {
        result += x[i] * y[i];
    }

    auto alpha = bulk::foldl(result,
      [](auto& lhs, auto rhs) { lhs += rhs; });

    return alpha;
}
```

Our template function `dot` takes two vectors, `x` and `y`, as input. The local results are stored in a distributed variable `result`, defined in the same world as the vectors. By a local computation, we set the local image of `result` to the correct value. Next, we call `foldl` on the distributed array `result`, and as binary operation we use a anonymous function that adds a value to another one. Finally, we return the result of this computation.

There are other ways in which we could have implemented this algorithm. The communication pattern used (an all-to-all broadcast followed by a reduction) is so common that has a special implementation in Bulk, but the communication could also have been done manually. In any case, the end result is a generic parallel inner product that works on a vector regardless of its element type `T`, its size, or the partitioning that is employed.

# `psc::sort`: sorting a vector<a id="sec-4"></a>

Next, we consider an example that is much more involved &#x2013; sorting a vector. While for the inner product algorithm we did not need to assume a specific partitioning (only that both vectors involved employed the same one), we *will* choose a specific partitioning for sorting: a block partitioning. The following example shows how the parallel sorting algorithm is called by a user.

```cpp
auto pi = bulk::block_partitioning<1>({size}, {p});
auto v = psc::vector<int>(world, pi);
psc::sort(v);
```

The parallel sorting algorithm we will implement is a distributed version of regular sample sorting. When the algorithm terminates, the result is a coarray where each element has roughly the same number of elements, every local array is sorted, and all elements on processor \(s > t\) are bigger than those on \(t\). The algorithm consists of the following steps.

1.  **Sorting the local vector**. First, the local elements are sorted using any sequential sorting algorithm.
2.  **Choosing and communicating `p` samples**. Next, we identify \(p\) equidistant samples. These \(p\) elements are broadcast to all other processors.
3.  **Finding global splitters**. Every processor now has \(p^2\) samples, which are sorted sequentially. From these samples, \(p\) *global* equidistant samples are taken. These global samples define the *splitters*. A processor \(s\) is responsible for a chunk of elements \(S_s \leq x < S_{s + 1}\), where \(S_i\) are the splitters.
4.  **Identify local blocks**. For all the local elements, the responsible processor is identified. Because the local array is sorted, the elements that have to be sent to a fixed processor \(t\) are a consecutive block of the local array.
5.  **Communicate blocks**. Every block is sent to the appropriate processor.
6.  **Merge the (individually sorted) blocks**. Each processor now has \(p\) blocks, which are all individually sorted. Using a merge sort routine the resulting sorted block can be computed efficiently.

These steps can be implemented in Bulk as follows. First, sorting the local vector can be done simply by using the sorting routine from the standard library.

```cpp
template <typename T>
bulk::coarray<T> sort(vector<T>& x) {
  // 1. Sorting the local vector
  std::sort(x.begin(), x.end());
  // ...
```

Next, we identify and communicate the local samples. Here, `sample` is a sequential routine that simply returns a number equidistant samples from an array. Also, `blocked_merge` takes an array consisting of a number of blocks that are individually sorted, and an array containing the sizes of the blocks, and sorts them together. In this case, after communicating, each processor has \(p\) sorted blocks that are all of size \(p\).

```cpp
// 2. Choosing and communicating samples
auto local_samples = sample(x, p);
auto samples = bulk::coarray<T>(world, p * p);
for (int t =0; t < p; ++t) {
    samples(t)[{s * p, (s + 1) * p}] = local_samples;
}
world.sync();
blocked_merge(samples, std::vector<int>(p, p));
```

Now each processor has the same \(p^2\) sorted samples, and we use it to compute the splitters. We set the upper limit for the final processor to \(\infty\).

```cpp
// 3. Finding global splitters
auto splitters = sample(samples, p);
splitters.push_back(std::numeric_limits<T>::max());
```

Next we want to split our local data up into chunks that each go to a different processor. We can write a simple sequential routine that does this, for example:

```cpp
// 4. Identify local blocks
int idx = 0;
int previous = 0;
std::vector<int> block_sizes(p);
for (int t = 0; t < p; ++t) {
    while (idx < (int)x.size() && x[idx] < splitters[t + 1]) {
        ++idx;
    }
    block_sizes[t] = idx - previous;
    previous = idx;
}
```

Now, `block_sizes(t)` is the size of the local chunk that is to be sent to processor `t`. Next, we communicate the chunks.

```cpp
// 5. Communicate chunks
auto block_starts = std::vector<int>(p);
std::partial_sum(block_sizes.begin(), block_sizes.end() - 1,
                 block_starts.begin() + 1);

auto q = bulk::queue<T[]>(world);
for (int t = 0; t < p; ++t) {
    q(t).send(std::vector(x.begin() + block_starts[t],
                          x.begin() + block_starts[t] + block_sizes[t]));
}
world.sync();
```

Next, we first compute some information about the blocks: their sizes, and where they start. We define a coarray with the appropriate local size, and copy the chunks in to it. Finally, we perform a `blocked_merge` on the received chunks, and return the resulting coarray.

```cpp
// 6. Merge the (individually sorted) blocks
auto receive_sizes = std::vector<int>(p);
std::transform(q.begin(), q.end(), receive_sizes.begin(),
               [](const auto& chunk) { return chunk.size(); });
auto receive_starts = std::vector<int>(p);
std::partial_sum(receive_sizes.begin(), receive_sizes.end() - 1,
                 receive_starts.begin() + 1);
auto total = std::accumulate(receive_sizes.begin(), receive_sizes.end(), 0);

auto ys = bulk::coarray<T>(world, total);

auto t = 0u;
for (const auto& chunk : q) {
  std::copy(chunk.begin(), chunk.end(), ys.begin() + receive_starts[t++]);
}

blocked_merge(ys, receive_sizes);

return ys;
```

Instead of a coarray, we could also return a vector with an *irregular block partitioning*, where the resulting sizes of the local array define the partitioning.

# `psc::matrix` class: distributed dense matrix<a id="sec-5"></a>

Our class for a distributed dense matrix is a lot like the class we defined for a distributed dense vector. The main difference is in the data, which is now two-dimensional instead of one-dimensional. We will also choose a associate a Cartesian partitioning, and assume our \(p\) processors are virtually laid out in a grid of size \(N \times M\). The head of the class looks as follows:

```cpp
namespace psc {

template <typename T>
class matrix {
  //...
```

We construct the matrix as follows:

```cpp
private:
  bulk::world& world_;
  bulk::cartesian_partitioning<2, 2>& partitioning_;
  bulk::coarray<T> data_;

public:
  matrix(bulk::world& world, bulk::cartesian_partitioning<2, 2>& partitioning,
         T value = 0)
      : world_(world), partitioning_(partitioning),
        data_(world_, partitioning.local_count(world_.rank()), value) {}
// ...
```

Note the Cartesian partitioning which has a `2` dimensional data space as well as a `2` dimensional grid space. Next, we provide a limited number of member functions:

```cpp
T& at(bulk::index_type<2> index);

auto& partitioning();
auto& world();

auto rows() { return partitioning_.global_size()[0]; }
auto cols() { return partitioning_.global_size()[1]; }
```

Here, `at` provides a reference to the matrix element with local index `index`. We provide `rows` and `cols` function for convenience, which forward the information from the partitioning.

# `psc::lu`: parallel in-place LU decomposition<a id="sec-6"></a>

Now that we have a distributed matrix class, we are ready to implement a parallel LU decomposition routine. The parallel LU algorithm consists of the following steps.

1.  Find pivot
    1.  **Find, and communicate local maximum with processor column**.
    2.  **Find global maximum element**.
    3.  **Communicate global maximum with processor row**.
2.  Swap rows
    1.  **Communicate permutation update**.
    2.  **Update permutation vector**.
    3.  **Communicate rows r and k**.
    4.  **Swap rows r and k**.
3.  Matrix update
    1.  **Communicate pivot**.
    2.  **Scale column**.
    3.  **Communicate row k and column k**.
    4.  **Update bottom right block**.

We will use a two-dimensional processor numbering, obtained as follows:

```cpp
auto [s, t] = psi.multi_rank(world.rank()).get();
```

Here, `psi` is a `cartesian_partitioning`.

We will not discuss the majority of sequential computations here, but they are given in the full example source code. We will show the body of the loop over `k`, which ranges from `0` to `n - 1` where `n` is the matrix size. We assume the local maximum in column `k` is at row `r_s` and has value `local_max`. Then communicating with the local processor column can be done as follows:

```cpp
// (1) Communicate maximum element with local processor column
if (psi.owner(1, k) == t) {
    for (auto u = 0; u < psi.grid()[0]; ++u) {
        pivot_rows(psi.rank({u, t}))[s] = r_s;
        pivot_values(psi.rank({u, t}))[s] = local_max;
    }
}
```

Here, `pivot_rows` and `pivot_cols` are two coarrays that are used to store all the local values. From these local maximums, the global maximums are found, and these can be communicated to the local processor *row* as follows. Afterwards, all processors know the pivot row `r` (stored inside a `bulk::var<int>`).

```cpp
// (3) Communicate global maximum with local processor row
if (psi.owner(1, k) == t) {
    for (auto v = 0; v < psi.grid()[1]; ++v) {
        r(psi.rank({s, v})) = pivot_rows[best_index];
    }
world.sync();
```

We now want to swap rows `k` and `r`. First, we want to make sure our bookkeeping is up to date, and swap the components \(\pi_r\) and \(\pi_k\) of the permutation vector \(\pi\). We assume \(\pi\) is stored in a coarray that is distributed according to the distribution function of the \(0\text{-th}\) axis.

```cpp
// (4) Communicate permutation update
if (psi.owner(0, k) == s) {
    pi(psi.rank({psi.owner(0, r), t}))[psi.local(0, r)] =
        pi[psi.local(0, k)];
}
if (psi.owner(0, r) == s) {
    pi(psi.rank({psi.owner(0, k), t}))[psi.local(0, k)] =
        pi[psi.local(0, r)];
}
world.sync();
```

Now that the permutation bookkeeping is done, we swap rows `r` and `k`. For this, we have two coarrays, which use local indices, that act as a buffer.

```cpp
// (6) Communicate row k and r
if (psi.owner(0, k) == s) {
    for (auto j = 0; j < psi.local_size(1, t); ++j) {
        row_swap_k(psi.rank({psi.owner(0, r), t}))[j] =
            mat.at({psi.local(0, k), j});
    }
}
if (psi.owner(0, r) == s) {
    for (auto j = 0; j < psi.local_size(1, t); ++j) {
        row_swap_r(psi.rank({psi.owner(0, k), t}))[j] =
            mat.at({psi.local(0, r), j});
    }
}
```

We then swap the rows locally.

```cpp
// (7) Locally swap row k and r
if (psi.owner(0, k) == s) {
    for (auto j = 0; j < psi.local_size(1, t); ++j) {
        mat.at({psi.local(0, k), j}) = row_swap_r[j];
    }
}
if (psi.owner(0, r) == s) {
    for (auto j = 0; j < psi.local_size(1, t); ++j) {
        mat.at({psi.local(0, r), j}) = row_swap_k[j];
    }
}
```

After swapping, we know the pivot element. This element is used to scale column `k`, so the processor that owns it should communicate it to its processor column.

```cpp
// (8) Communicate pivot
if (psi.owner({k, k}) == world.rank()) {
    for (auto u = 0; u < psi.grid()[0]; ++u) {
        pivot(psi.rank({u, t})) = mat.at(psi.local({k, k}));
    }
}
world.sync();
```

Next, the column is scaled.

```cpp
// (9) Scale column
if (psi.owner(1, k) == t) {
    // local row index, local col size, loop
    for (auto i = ls[k]; i < psi.local_size(0, s); ++i) {
        mat.at({i, psi.local(1, k)}) /= pivot;
    }
}
```

For the matrix update, the row `k` and column `k`, to the right of and below the diagonal, should be communicated to the processors that require it. This is done using horizontal and vertical communication. Here, `col_k` and `row_k` are two coarrays that act as a buffer.

```cpp
// (10) Horizontal, communicate column k
if (psi.owner(1, k) == t) {
    for (auto v = 0; v < psi.grid()[1]; ++v) {
        auto target_rank = psi.rank({s, v});
        for (auto i = 0; i < psi.local_size(0, s); ++i) {
            col_k(target_rank)[i] = mat.at({i, psi.local(1, k)});
        }
    }
}
world.sync();

// (10b) Vertical, communicate row k
if (psi.owner(0, k) == s) {
    for (auto u = 0; u < psi.grid()[0]; ++u) {
        auto target_rank = psi.rank({u, t});
        for (auto j = 0; j < psi.local_size(1, t); ++j) {
            row_k(target_rank)[j] = mat.at({psi.local(0, k), j});
        }
    }
}
world.sync();
```

Finally, we finish our matrix update.

```cpp
// (11) Update bottom right block
for (auto i = ls[k]; i < psi.local_size(0, s); ++i) {
    for (auto j = qs[k]; j < psi.local_size(1, t); ++j) {
        mat.at({i, j}) -= col_k[i] * row_k[j];
    }
}
```
