# `bulk::util::flatten`

```cpp
template <int D>
int flatten(std::array<int, D> volume, std::array<int, D> idxs);
```

Flatten a multi-index in a D-dimensional volume.

## Template parameters

* `D` - the dimension of the index space

## Parameters

* `volume` - the size of the volume
* `idxs` - the multi-index
