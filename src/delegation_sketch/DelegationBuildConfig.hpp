#pragma once
#include "frequency_estimator/MacroPreprocessor.hpp"
#include "utils/ConfigPrinter.hpp"
#include <string>

#ifndef ALGORITHM
    #define ALGORITHM count_min
    #define ALGORITHM augmented_sketch
    #define ALGORITHM heavy_keeper
    #define ALGORITHM cuckoo_heavy_keeper
#endif

#ifndef MODE
    #define MODE count
    #define MODE heavy_hitter
#endif

#ifndef PARALLEL_DESIGN
    #define PARALLEL_DESIGN GLOBAL_HASHMAP
    #define PARALLEL_DESIGN QPOPSS
#endif

#ifndef EVALUATE_MODE
    #define EVALUATE_MODE throughput
    #define EVALUATE_MODE latency
    #define EVALUATE_MODE accuracy
#endif

#ifndef EVALUATE_ACCURACY_WHEN
    #define EVALUATE_ACCURACY_WHEN start
    #define EVALUATE_ACCURACY_WHEN ivl_end
#endif

#ifndef EVALUATE_ACCURACY_ERROR_SOURCES
    #define EVALUATE_ACCURACY_ERROR_SOURCES algo
    #define EVALUATE_ACCURACY_ERROR_SOURCES algo_df
    #define EVALUATE_ACCURACY_ERROR_SOURCES algo_df_continuous
#endif

#ifndef EVALUATE_ACCURACY_STREAM_SIZE
    #define EVALUATE_ACCURACY_STREAM_SIZE 10000000
#endif
struct DelegationBuildConfig {
    static constexpr std::string_view algorithm = STRINGIFYMACRO(ALGORITHM);
    static constexpr std::string_view mode = STRINGIFYMACRO(MODE);
    static constexpr std::string_view parallel_design = STRINGIFYMACRO(PARALLEL_DESIGN);
    static constexpr std::string_view evaluate_mode = STRINGIFYMACRO(EVALUATE_MODE);
    static constexpr std::string_view evaluate_accuracy_when = STRINGIFYMACRO(EVALUATE_ACCURACY_WHEN);
    static constexpr std::string_view evaluate_accuracy_error_sources = STRINGIFYMACRO(EVALUATE_ACCURACY_ERROR_SOURCES);
    static constexpr int evaluate_accuracy_stream_size = EVALUATE_ACCURACY_STREAM_SIZE;

    auto to_tuple() const {
        return std::make_tuple("algorithm", std::string(DelegationBuildConfig::algorithm), "mode", std::string(DelegationBuildConfig::mode), "evaluate_mode",
                               std::string(DelegationBuildConfig::evaluate_mode), "parallel_design", std::string(DelegationBuildConfig::parallel_design), "evaluate_accuracy_when",
                               std::string(DelegationBuildConfig::evaluate_accuracy_when), "evaluate_accuracy_error_sources",
                               std::string(DelegationBuildConfig::evaluate_accuracy_error_sources), "evaluate_accuracy_stream_size",
                               DelegationBuildConfig::evaluate_accuracy_stream_size);
    }

    static constexpr bool is_valid_combination() {
        return (evaluate_mode != "accuracy") || (evaluate_accuracy_when == "start" && evaluate_accuracy_error_sources == "algo") ||
               (evaluate_accuracy_when == "start" && evaluate_accuracy_error_sources == "algo_df") ||
               (evaluate_accuracy_when == "end" && evaluate_accuracy_error_sources == "algo") ||
               (evaluate_accuracy_when == "end" && evaluate_accuracy_error_sources == "algo_df") ||
               (evaluate_accuracy_when == "ivl" && evaluate_accuracy_error_sources == "algo_df_continuous");
    }

    static constexpr void validate() { static_assert(is_valid_combination(), "Invalid configuration"); }
    friend std::ostream &operator<<(std::ostream &os, const DelegationBuildConfig &config) {
        ConfigPrinter<DelegationBuildConfig>::print(os, config);
        return os;
    }
};
