#include <catch.hpp>
#include <bulk.hpp>

SECTION("initializing bulk", "[init]") {
bulk::spawn(bulk::processors_available(), [](int s, int p) {
    CHECK(s < p);
});	
}
