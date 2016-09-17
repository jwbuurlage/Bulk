#include <bulk/backends/epiphany/host.hpp>
#include <bulk/environment.hpp>
#include <catch.hpp>

constexpr int streamcount = 100;

TEST_CASE("streaming", "[bulk-streaming]") {
    SECTION("everything") {
        bulk::environment<bulk::epiphany::provider> env;

        int success_count = 0;
        env.provider().set_log_callback(
            [&success_count](int pid, const std::string& log) {
                if (log.substr(0, 7) == "SUCCESS")
                    success_count++;
                else
                    std::cout << "Core " << pid << ": " << log << std::endl;
            });

        int* buffer = new int[env.available_processors() * streamcount];

        for (int i = 0; i < env.available_processors(); ++i)
            for (int j = 0; j < streamcount; ++j)
                buffer[i * streamcount + j] = i + j;

        for (int i = 0; i < env.available_processors(); ++i) {
            env.provider().create_stream(&buffer[i * streamcount],
                                         streamcount * sizeof(int),
                                         streamcount * sizeof(int));
        }

        env.spawn(
            env.available_processors(),
            [](bulk::world<bulk::epiphany::backend>& world, int s, int p) {

// Assertion for inside an spmd section
// Currently its put inside here because then bulk-compile-tool will copy it to
// the kernel file
#define BULK_ASSERT(expr)                                                      \
    if (expr) {                                                                \
        world.log("SUCCESS");                                                  \
    } else {                                                                   \
        world.log("Assert failed on line %d: %s", __LINE__, #expr);            \
        world.abort();                                                         \
    }

                if (s == 0)
                    world.log("Testing opening same stream twice. This should "
                              "output errors.");
                world.sync();

                // Test both constructors
                // Test opening same stream twice
                // Test opening an open stream
                bulk::epiphany::stream s1(s);
                bulk::epiphany::stream s2;
                s2.open(s);
                // s1 should be open
                // s2 should not be open and have printed an error
                BULK_ASSERT(s1 && !s2);
                // now close s1 and open s2 again
                s1.close();
                s2.open(s);
                BULK_ASSERT(!s1 && s2);
                s2.open(s); // Should do nothing
                s1.open(s); // Should print error
                BULK_ASSERT(!s1 && s2);

                // TODO:
                // Test seeking
                // Test read async
                // Test write async
                // Test read sync
                // Test write sync
                // Test communication back to host
                // Test size larger than extmem capacity
                // - subtest: write a littlebit, then seek, then write again

                constexpr int streamcount = 100;
                int* buf = new int[streamcount];
                BULK_ASSERT(buf);

                int bytes_read = s2.read(buf, streamcount * sizeof(int), false);
                s2.close();

                BULK_ASSERT(bytes_read == streamcount * sizeof(int));

                for (int i = 0; i < streamcount; ++i)
                    buf[i] += s;

                s1.open(s);
                s1.write(buf, streamcount * sizeof(int), false);
                s1.close();

                delete[] buf;
            });

        CHECK(success_count == env.available_processors() * 5);

        int buffer_fails = 0;
        for (int i = 0; i < env.available_processors(); ++i)
            for (int j = 0; j < streamcount; ++j)
                if (buffer[i * streamcount + j] != 2 * i + j)
                    ++buffer_fails;

        CHECK(buffer_fails == 0);

        delete[] buffer;
    }
}
