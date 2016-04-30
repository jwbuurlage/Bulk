#include <iostream>

#include <bulk.hpp>

int main() {
    bulk::spawn(bulk::available_processors(), [](int s, int p) {
        bulk::var<int> a;

        bulk::put(bulk::next_processor(), s, a);
        bulk::sync();

        BULK_IN_ORDER(std::cout << s << " <- " << a.value() << std::endl;)

        bulk::future<int> b = bulk::get<int>(bulk::next_processor(), a);

        bulk::sync();

        //BULK_IN_ORDER(std::cout << "hi ()" << std::endl;)

        BULK_IN_ORDER(std::cout << s << " -> " << b.value() << std::endl;)
    });

    return 0;
}
