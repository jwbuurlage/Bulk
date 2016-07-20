#include <bulk/environment.hpp>
#include <bulk/epiphany/host.hpp>

int main(int argc, char** argv) {
    bulk::environment<bulk::epiphany::provider> env;
    if (!env.provider().is_valid())
        return 1;

    int count = 100;
    for (int i = 0; i < env.available_processors(); i++) {
        env.provider().create_stream(
            count * sizeof(int),
            [i, count](void* buf, int offset, int size_requested) -> int {
                // TODO
                int* buffer = (int*)buf;
                for (int j = 0; j < count; j++)
                    buffer[j] = i + j;
                return count * sizeof(int);
            });
    }

    env.spawn(env.available_processors(), "e_basic.elf");
    return 0;
}
