// STRINGIFY
#define STRINGIFY(x)      #x
#define STRINGIFYMACRO(y) STRINGIFY(y)

// DEBUG macro
#ifdef DEBUG
    #define DEBUG_PRINT(x) std::cout << "[DEBUG] " << x << std::endl;
#else
    #define DEBUG_PRINT(x)
#endif

// print macro
#define PRINT_MACRO(macro_name) std::cout << #macro_name << ": " << STRINGIFYMACRO(macro_name) << std::endl;

// compare string in preprocessor
#define TRUE         1
#define XEQUAL(a, b) a##_##b
#define EQUAL(a, b)  XEQUAL(a, b)

// define available algorithm
#define cuckoo_heavy_keeper_cuckoo_heavy_keeper                 TRUE
#define heavy_keeper_heavy_keeper                               TRUE
#define wide_heavy_keeper_wide_heavy_keeper                     TRUE
#define count_min_count_min                                     TRUE
#define augmented_sketch_augmented_sketch                       TRUE
#define stream_summary_space_saving_stream_summary_space_saving TRUE
#define simple_space_saving_simple_space_saving                 TRUE
#define heap_hashmap_space_saving_heap_hashmap_space_saving     TRUE
#define weighted_frequent_weighted_frequent                     TRUE
#define optimized_weighted_frequent_optimized_weighted_frequent TRUE

// define available mode
#define heavy_hitter_heavy_hitter TRUE
#define count_count               TRUE

// define avaiable parallel design
#define GLOBAL_HASHMAP_GLOBAL_HASHMAP TRUE
#define QPOPSS_QPOPSS                 TRUE
