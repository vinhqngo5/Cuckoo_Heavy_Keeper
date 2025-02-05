#include <iostream>

#include "delegation_sketch/DelegationBuildConfig.hpp"
#include "delegation_sketch/DelegationConfig.hpp"
#include "delegation_sketch/DelegationHeavyHitter.hpp"
#include "delegation_sketch/DelegationSketch.hpp"
#include "delegation_sketch/delegation_sketch_utils.hpp"
#include "frequency_estimator/AugmentedSketch.hpp"
#include "frequency_estimator/CountMinSketch.hpp"
#include "frequency_estimator/CuckooHeavyKeeper.hpp"
#include "frequency_estimator/FrequencyEstimatorConfig.hpp"
#include "frequency_estimator/HeapHashMapSpaceSaving.hpp"
#include "frequency_estimator/HeavyKeeper.hpp"
#include "frequency_estimator/MacroPreprocessor.hpp"
#include "heavy_hitter_app/AppConfig.hpp"

using namespace std;

#if EQUAL(ALGORITHM, count_min)
using FrequencyEstimatorConfig = CountMinConfig;
using FrequencyEstimator = CountMinSketch;
#elif EQUAL(ALGORITHM, augmented_sketch)
using FrequencyEstimatorConfig = AugmentedSketchConfig;
using FrequencyEstimator = AugmentedSketch;
#elif EQUAL(ALGORITHM, cuckoo_heavy_keeper)
using FrequencyEstimatorConfig = CuckooHeavyKeeperConfig;
using FrequencyEstimator = CuckooHeavyKeeper;
#elif EQUAL(ALGORITHM, heavy_keeper)
using FrequencyEstimatorConfig = HeavyKeeperConfig;
using FrequencyEstimator = HeavyKeeper;
#elif EQUAL(ALGORITHM, heap_hashmap_space_saving)
using FrequencyEstimatorConfig = SpaceSavingConfig;
using FrequencyEstimator = HeapHashMapSpaceSavingV2;
#endif

using DelegationConfigBasedOnMode = DelegationHeavyHitterConfig;
using MultithreadedApp = DelegationHeavyHitter<FrequencyEstimator>;

// assert that parser information is consistent
void assert_consistency(AppConfig &app_configs, DelegationConfigBasedOnMode &delegation_configs, FrequencyEstimatorConfig &frequency_estimator_configs);

int main(int argc, char **argv) {
    // init configs
    ConfigParser parser;

    // configs
    using AppConfig = ParallelAppConfig;
    AppConfig app_configs;
    DelegationConfigBasedOnMode delegation_configs;
    FrequencyEstimatorConfig frequency_estimator_configs;
    DelegationBuildConfig delegation_build_configs;

    // add parameters to the parser
    AppConfig::add_params_to_config_parser(app_configs, parser);
    FrequencyEstimatorConfig::add_params_to_config_parser(frequency_estimator_configs, parser);
    DelegationConfigBasedOnMode::add_params_to_config_parser(delegation_configs, parser);
    DelegationBuildConfig::validate();

    // run the parser
    if (argc == 2 && (strncmp(argv[1], "--help", 6) == 0 || strncmp(argv[1], "-h", 2) == 0)) {
        parser.PrintUsage();
        exit(0);
    }

    if (argc == 2 && (strncmp(argv[1], "--generate-doc", 6) == 0 || strncmp(argv[1], "-h", 2) == 0)) {
        fprintf(stdout, "Generating the parameter list in markdown format for use in the documentation.\n\n");
        parser.PrintMarkdown();
        exit(0);
    }

    Status s = parser.ParseCommandLine(argc, argv);
    if (!s.IsOK()) {
        fprintf(stderr, "%s\n", s.ToString().c_str());
        exit(-1);
    }

    // print the configs (at the beginning), will be printed again while running the program so that it can be logged separately
    cout << DelegationBuildConfig();
    cout << app_configs;
    cout << delegation_configs;
    cout << frequency_estimator_configs;

    // generate the dataset
    Relation *r1 = generate_relation(app_configs);

    for (int i = 0; i < app_configs.NUM_RUNS; i++) {
        // init vector of frequency_estimator objects
        vector<FrequencyEstimator> frequency_estimators;
        for (int i = 0; i < app_configs.NUM_THREADS; i++) { frequency_estimators.push_back(FrequencyEstimator(frequency_estimator_configs)); }

        // init delegation sketch context
        atomic<bool> START_BENCHMARK = false;
        atomic<bool> START_ACCURACY_EVALUATION = false;
        DelegationSketchContext delegation_sketch_context(app_configs, delegation_configs, r1, START_BENCHMARK, START_ACCURACY_EVALUATION);

        // print important configs
        string log_output_path = create_file_path_from_context(delegation_sketch_context, "_log");

        // Create an output file stream if output_file_path is specified
        ofstream output_file;
        if (!log_output_path.empty()) {
            output_file.open(log_output_path);
            if (!output_file.is_open()) {
                cerr << "Failed to open output file: " << log_output_path << endl;
                // Continue execution, but only output to cout
            }
        }

        // Function to print to both cout and file if specified
        auto print = [&output_file](const auto &x) {
            cout << x;
            if (output_file.is_open()) output_file << x;
        };

        print(DelegationBuildConfig());
        print(app_configs);
        print(delegation_configs);
        print(frequency_estimator_configs);

        // start threads
        MultithreadedApp *delegation_sketch = start_threads<FrequencyEstimator>(delegation_sketch_context, frequency_estimators);

        // replace _log with _delegation
        size_t pos = log_output_path.find("_log");
        string delegation_output_file_path = log_output_path;
        if (pos != string::npos) { delegation_output_file_path.replace(pos, 4, "_delegation"); }

        string heavyhitter_output_file_path = log_output_path;
        if (pos != string::npos) { heavyhitter_output_file_path.replace(pos, 4, "_heavyhitter"); }

        // print stats
        print_stats_for_delegation_sketch(delegation_sketch_context, delegation_sketch, delegation_output_file_path);
        print_stats_for_heavy_hitters<FrequencyEstimator, int>(delegation_sketch_context, delegation_sketch, heavyhitter_output_file_path);
    }

    return 0;
}
