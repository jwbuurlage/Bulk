#include <bulk/environment.hpp>
#include <bulk/backends/epiphany/host.hpp>
#include <catch.hpp>

TEST_CASE("lambda kernels", "[bulk-compile-tool]") {
    bulk::environment<bulk::epiphany::provider> env;

    //REQUIRE(env.provider().is_valid());

    int success_count;

    env.provider().set_log_callback(
        [&success_count](int pid, const std::string& log) {
            if (log.substr(0, 7) == "SUCCESS")
                success_count++;
            else
                std::cout << "Core " << pid << ": " << log << std::endl;
        });

    SECTION("print") {
        success_count = 0;
        env.spawn(env.available_processors(), [](bulk::world<bulk::epiphany::backend>& world, int s, int p) {
            bulk::epiphany::print("SUCCESS");
        });
        CHECK(success_count == env.available_processors());
    }

    SECTION("auto_world") {
        success_count = 0;
        env.spawn(env.available_processors(), [](auto& world, int s, int p) {
            bulk::epiphany::print("SUCCESS");
        });
        CHECK(success_count == env.available_processors());
    }
}
