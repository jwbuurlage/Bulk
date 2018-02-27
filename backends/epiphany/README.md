# Bulk Epiphany backend

## Epiphany backend build instructions

Build the compile tool:

    cd backends/epiphany/bulk-compile-tool/
    make

Add compile-tool to path:

    export PATH = "$PATH:path-to-repository/backends/epiphany/bulk-compile-tool/bin"

Build epiphany backend library

    cd backends/epiphany
    make

Build examples

    cd backends/epiphany/examples
    make

Run an example

    backends/epiphany/examples/bin/unittests


## Epiphany backend limitations

The main features of Bulk such as variables, coarrays and message queues are all implemented for the Epiphany backend and work as expected with the same syntax. The experimental partitioning features have not been implemented on the Epiphany platform.

Due to the small memory size of the Epiphany cores, some C++ code will not compile because the generated code is too big. This is the case, for example, when using arrays within Bulk message queues. When this is the case, the linker will output a message about overflowing `INTERNAL_RAM`.

