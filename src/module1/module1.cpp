#include "module1.hpp"
#include <iostream>

void Module1::run_public_module1() {
    std::cout << "Running public module1" << std::endl;
}

void Module1::run_private_module1() {
    std::cout << "Running private module1" << std::endl;
}