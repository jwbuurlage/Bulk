Variables
=========

Now that we are able to set up the environment, and gained some
familiarity with the *world* object that can be used to communicate with
other processors, we are ready to discuss communication between
processors. The most fundamental way of communicating between processors
is using *distributed variables*. The variables can be created as
follows:

```
auto x = bulk::var<int>(world);
```

Here we create a distributed variable that holds an integer. A
distributed variable exists on every processor, but can have different
values on different processors. These different local 'copies' are
referred to as *images* of the variable. The variable lives within the
*world* of the current SPMD section, and we explicitely write this by
passing the world object as a parameter to the variable creation
function.

While `x` refers to the *local image* of a variable, it is
identified with images on remote processors by the order in which
variables are constructed (which is possible because of the SPMD nature
of Bulk programs). This allows us to *write to* and *read from* remote
images of a variable by simply passing `x` to communication
functions.

Bulk synchronous communication
------------------------------

The main way to manipulate remote images of variables is using the
communication primitives `bulk::put` and
`bulk::get` for writing and reading respectively. For
example, to write the value `1` to the remote image held by
the next logical processor, we write:

``` 
bulk::put(world.next_rank(), 1, x);
```

To obtain the value of a remote image we write:

``` 
auto y = bulk::get(world.next_rank(), x);
```

!!! note
    Equivalently, we can use the short-hand syntax: `x(world.next_rank()) = 1` for putting, and `auto y = x(world.next_rank()).get()` for getting.

<center>
![image](images/variable.png)
</center>

Initially, communication is only staged. This means that the values are not
valid immediately after the execution of the communication primitives.
Instead, they are available in the next 'section' of the program, known
as a (BSP) **superstep**. Supersteps can be viewed as the section of the
program within successive calls to `world.sync()`. This
barrier synchronization asserts that all processors have reached that
point of the program, and resolves all outstanding communication such as
those staged by calls to put and get. After the barrier synchronization
returns, all communication staged in the previous superstep is
guaranteed to have occurred.

<center>
![The structure of a superstep. First, computations are performed.
Optionally, communication is staged for later execution. After each core
finishes their computations, outstanding communications are performed.
This process is repeated until the SPMD section
terminates.](images/superstep.png)
</center>

In case of *put*, the remote image now contains the value written to it
(assuming that the local processor is the only one who wrote to that
specific remote image). For read requests using *get*, it is slightly
more complicated. The type of `y` is a
`bulk::future<T>`. A future object is a placeholder, that
will contain the correct value in the next superstep. This value can be
obtained after the synchronization using:

```
auto value = y.value();
```

This way of communicating is particularly useful when dealing with
simple data objects. If instead we deal with distributed array-like
objects, we recommend using *coarrays*, which are introduced in the
next section.
