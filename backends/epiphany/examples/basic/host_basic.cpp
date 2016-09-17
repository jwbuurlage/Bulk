#include <bulk/environment.hpp>
#include <bulk/backends/epiphany/host.hpp>
#include <iostream>
#include <vector>

int main(int argc, char** argv) {
    bulk::environment<bulk::epiphany::provider> env;
    if (!env.provider().is_valid())
        return 1;

    int count = 32;
    std::vector<std::vector<int>> data(env.available_processors());

    for (int i = 0; i < env.available_processors(); i++) {
        data[i].resize(count);
        for (int j = 0; j < count; j++)
            data[i][j] = i + j;
    }

    for (int i = 0; i < env.available_processors(); i++)
        env.provider().create_stream(&data[i][0], count * sizeof(int),
                                     count * sizeof(int));

    env.spawn(env.available_processors(), "e_basic.elf");

    for (int i = 0; i < env.available_processors(); i++) {
        std::cout << "Result from processor " << i << ":\n";
        for (int j = 0; j < count; j++)
            std::cout << data[i][j] << ' ';
        std::cout << '\n';
    }

    return 0;
}
