#include "concurrent_data_structure/MichaelScottQueue.hpp"
#include "example_queue_common.hpp"
#include <iostream>

int main() {
    test_concurrent_queue<MichaelScottQueue, int>(4);

    return 0;
}