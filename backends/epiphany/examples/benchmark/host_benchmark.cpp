#include <bulk/environment.hpp>
#include <bulk/backends/epiphany/host.hpp>
#include <iostream>
#include <vector>

int main(int argc, char** argv) {
    bulk::environment<bulk::epiphany::provider> env;
    if (!env.provider().is_valid())
        return 1;

    env.spawn(env.available_processors(), "e_benchmark.elf");

    return 0;
}
