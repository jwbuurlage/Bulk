# Epiphany backend for Bulk

The main features of Bulk such as variables, coarrays and message queues are all implemented for the Epiphany backend and work as expected with the same syntax.

_Warning_: due to the small memory size of the Epiphany cores, certain C++ code will not compile because the generated code is too big. This happens, for example, when using arrays within Bulk message queues.
When this is the case, the linker will output a message about overflowing `INTERNAL_RAM`.

## Usage

First, Build the compile tool:

    cd backends/epiphany/bulk-compile-tool/
    make

Next, add the compile-tool to path:

    export PATH = "$PATH:path-to-repository/backends/epiphany/bulk-compile-tool/bin"

To build the Epiphany backend library, do

    cd backends/epiphany
    make

To build the Epiphany examples, do

    cd backends/epiphany/examples
    make

To run an example, use e.g.

    backends/epiphany/examples/bin/unittests




