#include <bulk/bulk.hpp>

#include "bulk_test_common.hpp"
#include "set_backend.hpp"

int total = 0;
int success = 0;

extern void test_initialization();
extern void test_communication();
extern void test_partitioning();
extern void test_algorithm();
extern void test_array();

environment env;

int main() {
  test_initialization();
  test_communication();
  test_partitioning();
  test_algorithm();
  test_array();

  BULK_FINALIZE_TESTS(env);
}
