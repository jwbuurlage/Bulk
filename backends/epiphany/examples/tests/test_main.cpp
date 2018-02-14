#include <bulk/bulk.hpp>
#include "bulk_test_common.hpp"
#include "set_backend.hpp"

extern void test_initialization();
extern void test_communication();
extern void test_partitioning();

environment env;

int main() {
    test_initialization();
    test_communication();
    //test_partitioning();
}
