#include <bulk/backends/mpi/mpi.hpp>
#include <bulk/bulk.hpp>

#include "bulk_mpi_test_common.hpp"

int total = 0;
int success = 0;

extern void test_initialization();
extern void test_communication();

bulk::environment<provider> env = {};

int main() {
    test_initialization();
    test_communication();

    BULK_FINALIZE_TESTS(env);
}
