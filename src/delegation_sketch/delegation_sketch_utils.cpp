#include "delegation_sketch_utils.hpp"

using std::default_random_engine;

unsigned long *seed_rand() {
    unsigned long *seeds = (unsigned long *) calloc(3, sizeof(unsigned long));
    seeds[0] = getticks() % 123456789;
    seeds[1] = getticks() % 362436069;
    seeds[2] = getticks() % 521288629;
    return seeds;
}

unsigned long xorshf96(unsigned long *x, unsigned long *y, unsigned long *z) {
    unsigned long t;
    (*x) ^= (*x) << 16;
    (*x) ^= (*x) >> 5;
    (*x) ^= (*x) << 1;

    t = *x;
    (*x) = *y;
    (*y) = *z;
    (*z) = t ^ (*x) ^ (*y);

    return *z;
}

bool should_perform_query(unsigned long *seeds, double query_rate) { return (delegation_query_random(&(seeds[0]), &(seeds[1]), &(seeds[2])) % 10000) < query_rate; }

struct timeval global_timer_start, global_timer_stop;

void start_time() { gettimeofday(&global_timer_start, NULL); }

void stop_time() { gettimeofday(&global_timer_stop, NULL); }

unsigned long get_time_ms() { return (global_timer_stop.tv_sec - global_timer_start.tv_sec) * 1000 + (global_timer_stop.tv_usec - global_timer_start.tv_usec) / 1000; }

void setaffinity_oncpu(unsigned int cpu) {
    cpu_set_t cpu_mask;
    CPU_ZERO(&cpu_mask);
    CPU_SET(cpu, &cpu_mask);

    int err = sched_setaffinity(0, sizeof(cpu_set_t), &cpu_mask);
    if (err) {
        perror("sched_setaffinity");
        exit(1);
    }
}

unsigned short precomputed_mods[512];

void precompute_mods(int num_threads) {
    int c = 0;
    for (int i = 0; i < 512; i++) {
        precomputed_mods[i] = c;
        c++;
        c = c % num_threads;
    }
}

// inline int find_owner(unsigned int key) { return precomputed_mods[key & 511]; }
