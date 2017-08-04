# Bulk vs BSPlib

In this section we highlight some of the differences between Bulk and BSPlib.

## No global state

In BSPlib, the SPMD section is delimited using `bsp_begin()` and `bsp_end()` calls. In Bulk,
the SPMD section is passed as an argument to the `environment::spawn` function.

```cpp
// BSPlib
#include <bsp.h>

int main() {
    bsp_begin(bsp_nprocs());
    int s = bsp_pid();
    int p = bsp_nprocs();
    printf("Hello World from processor %d / %d", s, p);
    bsp_end();

    return 0;
}

// Bulk
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

When using BSPlib, all calls to the parallel system happen globally. In particular, calls such as `bsp_sync()` and
`bsp_push_reg(...)` implicitly modify some global state.

In Bulk, the state is captured in a `world` object. Besides the well-known
reasons not to use global state, this allows for multi-layered parallelism. For
example, one world could contain the nodes in an MPI cluster, while another
world represents the multi-core system on such a node.

## Registration of variables

Variables and message queues are registered and initialized upon creation, and deregistered when
they go out of scope. This means that explicit registration calls are no longer
necessary. Compare:

```cpp
// BSPlib
int x = 0;
bsp_push_reg(&x, sizeof(int));
bsp_sync();
...
bsp_pop_reg(&x);

// Bulk
auto x = bulk::var<int>(world);
```

## Communication

Communication using simple distributed variables is expressed more compactly in
Bulk. Compare:

```cpp
// BSPlib
int b = 3;
bsp_put((s + 1) % p, &b, &x, 0, sizeof(int));

int c = 0;
bsp_get((s + 1) % p, &x, 0, &c, sizeof(int));

bsp_sync();

// Bulk
x(world.next_rank()) = 3;
auto c = x(world.next_rank()).get();

world.sync();
```

## Arrays

Bulk treats distributed arrays differently from simple values. This is done using coarrays. Compare:
```cpp
// BSPlib
int* xs = malloc(10 * sizeof(int));
bsp_push_reg(xs, 10 * sizeof(int));
bsp_sync();

int ys[3] = {1, 2, 3};
bsp_put((s + 1) % p, ys, xs, 2, 3 * sizeof(int));
int z = 5;
bsp_put((s + 1) % p, &z, xs, 0, sizeof(int));

bsp_sync();

...

bsp_pop_reg(xs);
free(xs);

// Bulk
auto xs = bulk::coarray<int>(world, 10);
xs(world.next_rank())[{2, 5}] = {2, 3, 4};
xs(world.next_rank())[0] = 5;

world.sync();
```

## Message passing

In BSPlib, messages consist of a _tag_ and a _content_. In Bulk, we don't force this message structure, but we do support it. Compare:

```cpp
// BSPlib
int s = bsp_pid();
int p = bsp_nprocs();

int tagsize = sizeof(int);
bsp_set_tagsize(&tagsize);
bsp_sync();

int tag = 1;
int payload = 42 + s;
bsp_send((s + 1) % p, &tag, &payload, sizeof(int));
bsp_sync();

int packets = 0;
int accum_bytes = 0;
bsp_qsize(&packets, &accum_bytes);

int payload_in = 0;
int payload_size = 0;
int tag_in = 0;
for (int i = 0; i < packets; ++i) {
    bsp_get_tag(&payload_size, &tag_in);
    bsp_move(&payload_in, sizeof(int));
    printf("payload: %i, tag: %i", payload_in, tag_in);
}

// Bulk
auto s = world.rank();
auto p = world.active_processors();

auto q = bulk::queue<int, int>(world);
q(world.next_rank()).send(1, 42 + s);
world.sync();

for (auto [tag, content] : queue) {
    world.log("payload: %i, tag: %i", content, tag);
}
```
In addition, Bulk supports sending arbitrary data either using custom structs, or by composing messages on the fly. For example, to send a 3D tensor element with indices and its value, we can write:
```cpp
auto q = bulk::queue<int, int, int, float>(world);
q(world.next_rank()).send(1, 2, 3, 4.0f);
world.sync();

for (auto [i, j, k, value] : queue) {
    world.log("element: A(%i, %i, %i) = %f", i, j, k, value);
}
```

Also, multiple queues can be constructed, which eliminates a common use case for using tags.

## Other

The algorithmic skeletons in Bulk allow common patterns to be implemented in
fewer lines compared to BSPlib. As an example, we show how to find the maximum
element over all processors.

```cpp
auto maxs = bulk::gather_all(world, max);
max = *std::max_element(maxs.begin(), maxs.end());
```

Compare this to the way this is done when using BSPlib:

```cpp
int* global_max = malloc(sizeof(int) * bsp_nprocs());
bsp_push_reg(global_max, sizeof(int) * bsp_nprocs());

for (int t = 0; t < p; ++t) {
    bsp_put(t, &max, global_max, bsp_pid(), sizeof(int));
}
bsp_sync();

for (int t = 0; t < p; ++t) {
    if (max < global_max[t]) {
        max = global_max[t];
    }
}

bsp_pop_reg(global_max);
free(global_max);
```
