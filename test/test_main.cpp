#include "bulk_test_common.hpp"
#include "set_backend.hpp"
#include <bulk/bulk.hpp>

int total = 0;
int success = 0;

extern void test_initialization();
extern void test_communication();
extern void test_partitioning();

environment env;

int main() {
    test_initialization();
    test_communication();
    test_partitioning();

    BULK_FINALIZE_TESTS(env);
}
