#include <bulk/bulk.hpp>
#include <iostream>

#include "../mpi.hpp"

int main() {
  bulk::mpi::environment env;

  env.spawn(env.available_processors(), [](auto& world) { world.log("Hi!"); });

  return 0;
}
