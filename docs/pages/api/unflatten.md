# `bulk::util::unflatten`

```cpp
template <int D>
std::array<int, D> unflatten(std::array<int, D> volume, int flattened);
```

Unflatten a multi-index in a D-dimensional volume.

## Template parameters

* `D` - the dimension of the index space

## Parameters

* `volume` - the size of the volume
* `idxs` - the multi-index
