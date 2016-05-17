# `bulk::environment::spawn`

```cpp
void spawn(int processors, std::function<void(int, int)> spmd);
```

Start an SPMD section on a given number of processors.

## Parameters

* `processors` - the number of processors to run the spmd section on.
* ` spmd` - the SPMD function that gets run on each (virtual) processor. The world of the processor will be passed as the first argument. The local processor id: `world.processor_id()`, will be passed as the second argument to `spmd`, while the number of active processors: `world.active_processors()`, will be passed as the third argument.
