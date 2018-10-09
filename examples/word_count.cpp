#include <fstream>
#include <functional>
#include <map>
#include <string>

#include "bulk/bulk.hpp"

#include "set_backend.hpp"

// This example shows how to perform a word count of a certain text (in this
// case, the first chapter of Alice in Wonderland) in parallel.

int main() {
    environment env;

    env.spawn(env.available_processors(), [](bulk::world& world) {
        auto s = world.rank();
        auto p = world.active_processors();

        // We create a queue that holds the individual words of the text
        auto words = bulk::queue<std::string>(world);

        // The first processor is the master; it reads the file and sends the
        // individual words around
        if (s == 0) {
            // We run this example from the project root, and open the text
            auto f = std::fstream("examples/data/alice.txt");

            // We loop over all the words in the text
            std::string word;
            while (f >> word) {
                // For each word, we compute a hash. This hash decides the
                // receiving processor, who is responsible for counting the
                // occurences of that word. This is called the _map_ step
                words(std::hash<std::string>{}(word) % p).send(word);
            }
        }

        world.sync();

        // This next block groups the words together, and corresponds to the
        // usual reduce phase.
        auto counts = std::map<std::string, int>{};
        for (auto word : words) {
            if (counts.find(word) != counts.end()) {
                counts[word]++;
            } else {
                counts[word] = 1;
            }
        }

        // We send the results back to the master
        auto report = bulk::queue<std::string, int>(world);
        for (auto [word, count] : counts) {
            report(0).send(word, count);
        }

        world.sync();

        // The master sorts the incoming reports, and logs the resulting counts
        std::sort(report.begin(), report.end(), [](auto lhs, auto rhs) {
            return std::get<1>(lhs) < std::get<1>(rhs);
        });

        for (auto [word, count] : report) {
            world.log("%s: %d", word.c_str(), count);
        }
    });

    return 0;
}
