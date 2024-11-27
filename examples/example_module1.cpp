#include "module1/module1.hpp"

#include <chrono>   // Add this line
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>   // Add this line

auto main(int argc, char *argv[]) -> int {
    int a_value = 0;
    bool a_set = false;

    // Loop through all arguments to find --a
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "--a" && i + 1 < argc) {
            a_value = std::atoi(argv[++i]);
            a_set = true;
        }
    }

    if (a_set) {
        std::cout << "Running module1 example with arg: " << a_value
                  << std::endl;
    } else {
        std::cout << "Running module1 example with default settings..."
                  << std::endl;
    }

    // Running module1 example
    Module1 module1;
    module1.run_public_module1();

    return 0;
}
