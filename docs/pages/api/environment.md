# `bulk::environment`

Defined in header `<bulk/environment.hpp>`.

```cpp
template<class Provider>
class environment;
```

`bulk::environment` encodes the parallel environment of this layer, and provides information on the system.

## Template parameters

* `Provider` - a low level implementation of the transport and system primitives. See also [`bulk::provider`](api/provider.md).

## Member types

| **Member type**  | **Definition**                  |
|------------------|---------------------------------|
| `world_provider` | `Provider::world_provider_type` |
| `world_type`     | `world<world_provider>`         |

## Member functions

|                                                               |                                                       |
|---------------------------------------------------------------|-------------------------------------------------------|
| **Initialization**                                            |                                                       |
| [`spawn`](environment/spawn.md)                               | spawns a spmd section on a given number of processors |
| **System information**                                        |                                                       |
| [`available_processors`](environment/available_processors.md) | returns the number of available processors            |
| **Low-level access**                                          |                                                       |
| [`provider`](environment/provider.md)                         | returns the provider                                  |
