#include "frequency_estimator/sequential_frequency_estimator_test_utils.hpp"
#include "prif/PRIF.hpp"
#include "prif/prif_test_utils.hpp"
#include "utils/ConfigParser.hpp"
#include "utils/ConfigPrinter.hpp"

#include <iostream>
using namespace std;

#if EQUAL(ALGORITHM, weighted_frequent)
using FrequencyEstimatorConfig = WeightedFrequentConfig;
using FrequencyEstimator = WeightedFrequent;
#elif EQUAL(ALGORITHM, optimized_weighted_frequent)
using FrequencyEstimatorConfig = OptimizedWeightedFrequentConfig;
using FrequencyEstimator = OptimizedWeightedFrequent;
#else
#endif

// global variables to start/stop benchmark
atomic<bool> START_BENCHMARK = true;

// configs
using AppConfig = ParallelAppConfig;
AppConfig app_configs;
PRIFConfig prif_configs;
FrequencyEstimatorConfig frequency_estimator_configs;

int main(int argc, char **argv) {
    // init configs
    ConfigParser parser;

    // add parameters to the parser
    AppConfig::add_params_to_config_parser(app_configs, parser);
    PRIFConfig::add_params_to_config_parser(prif_configs, parser);
    FrequencyEstimatorConfig::add_params_to_config_parser(frequency_estimator_configs, parser);

    // run the parser
    if (argc == 2 && (strncmp(argv[1], "--help", 6) == 0 || strncmp(argv[1], "-h", 2) == 0)) {
        parser.PrintUsage();
        return 0;
    }

    // parse the command line arguments
    Status s = parser.ParseCommandLine(argc, argv);
    if (!s.IsOK()) {
        fprintf(stderr, "%s\n", s.ToString().c_str());
        exit(-1);
    }

    // print the configs
    cout << frequency_estimator_configs;
    cout << app_configs;

    // run the test
    test<FrequencyEstimatorConfig>(app_configs, prif_configs, frequency_estimator_configs);

    return 0;
}