# `bulk::world::split`

```cpp
std::unique_ptr<bulk::world> split(int part)
```

Split the world into a number of parts. Every processor in the same part after
splitting get assigned consecutive 0-based ranks in the subworld in
ascending order from their rank in the ambient world.

The subworld is returned as a unique pointer, to ensure proper clean up
when it is no longer needed.

This allows synchronization and communication between subsets of processors.

There is no limit on the number of splits, and resources get cleaned up
automatically when subworlds are no longer used.

## Example

```cpp
auto sub = world.split(s % 2);

// Ranks get reordered according to their original ranks in 'world'
assert(sub->rank() == world.rank() / 2);

// Otherwise, sub acts just like world
auto x = bulk::var<int>(*sub, s);
auto y = x(sub->next_rank()).get();
sub->sync();

// The next rank in the subworld, is the next of the next rank
// in the original 'world'.
assert(y.value() == ((s + 2) % p));
```
