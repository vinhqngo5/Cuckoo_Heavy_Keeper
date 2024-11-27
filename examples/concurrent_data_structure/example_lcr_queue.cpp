#include "concurrent_data_structure/LCRQueue.hpp"
#include "example_queue_common.hpp"
#include <iostream>

int main() {

    using int_pointer = int *;
    test_concurrent_queue<LCRQueue, int *>(4);

    return 0;
}