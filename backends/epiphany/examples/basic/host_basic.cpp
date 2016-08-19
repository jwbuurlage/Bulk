#include <bulk/environment.hpp>
#include <bulk/epiphany/host.hpp>

int main(int argc, char **argv)
{
    bulk::environment<bulk::epiphany::provider> env;
    if (!env.provider().is_valid())
        return 1;
    env.spawn(env.available_processors(), "e_basic.elf");
    return 0;
}
