# `bulk::environment::spawn`

```cpp
void spawn(int processors, std::function<void(bulk::world&)> spmd);
```

Start an SPMD section on a given number of processors.

## Parameters

* `processors` - the number of processors to run the SPMD section on.
* ` spmd` - the SPMD function that gets run on each (virtual) processor. A reference to the world of the processor will be passed as the first and only argument.
