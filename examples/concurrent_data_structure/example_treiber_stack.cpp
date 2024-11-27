#include "concurrent_data_structure/TreiberStack.hpp"
#include "example_queue_common.hpp"
#include <iostream>

int main() {
    test_concurrent_queue<TreiberStack, int>(4);

    return 0;
}