#pragma once
#include "params.hpp"
#include <stdint.h>
struct Bucket {
    uint32_t key[COUNTER_PER_BUCKET];
    uint32_t val[COUNTER_PER_BUCKET];
};