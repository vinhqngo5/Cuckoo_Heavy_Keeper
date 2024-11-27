
#pragma once

#include "utils/ConfigParser.hpp"
#include "utils/ConfigPrinter.hpp"
#include <getopt.h>
#include <string>

using std::string;

struct CommonAppConfig {
    string MODE;
    int NUM_RUNS;
    int LINE_READ;
    int DOM_SIZE;
    int tuples_no;
    int DIST_TYPE;
    double DIST_PARAM;
    double DIST_SHUFF;
    string DATASET;
    float THETA;
    int DURATION;

    static void add_params_to_config_parser(CommonAppConfig &app_config, ConfigParser &parser) {
        // App configs prefix will be "app."
        parser.AddParameter(new StringParameter("app.mode", "count", &app_config.MODE, false, "Mode of the application: count/top-k/heavy_hitter"));
        parser.AddParameter(new IntParameter("app.num_runs", "10", &app_config.NUM_RUNS, false, "Number of runs for the application"));
        parser.AddParameter(new IntParameter("app.line_read", "1000000", &app_config.LINE_READ, false, "Number of lines to read from the input file"));
        parser.AddParameter(new IntParameter("app.dom_size", "1000000", &app_config.DOM_SIZE, false, "Domain size for the dataset"));
        parser.AddParameter(new IntParameter("app.tuples_no", "6000000", &app_config.tuples_no, false, "Number of tuples for the dataset"));
        parser.AddParameter(new IntParameter("delegation.dist_type", "1", &app_config.DIST_TYPE, false, "Distribution type for the dataset"));
        parser.AddParameter(new DoubleParameter("app.dist_param", "1.3", &app_config.DIST_PARAM, false, "Distribution parameter for the dataset"));
        parser.AddParameter(new DoubleParameter("app.dist_shuff", "0", &app_config.DIST_SHUFF, false, "Distribution shuffle for the dataset"));
        parser.AddParameter(new StringParameter("app.dataset", "zipf", &app_config.DATASET, false, "Dataset: CAIDA/zipf"));
        parser.AddParameter(new FloatParameter("app.theta", "0.01", &app_config.THETA, false, "Theta value for the finding heavy hitters"));
        parser.AddParameter(new IntParameter("app.duration", "1", &app_config.DURATION, false, "Duration of the benchmark"));
    }

    auto to_tuple() const {
        return std::make_tuple("MODE", MODE, "NUM_RUNS", NUM_RUNS, "LINE_READ", LINE_READ, "DOM_SIZE", DOM_SIZE, "tuples_no", tuples_no, "DIST_TYPE", DIST_TYPE, "DIST_PARAM",
                               DIST_PARAM, "DIST_SHUFF", DIST_SHUFF, "DATASET", DATASET, "THETA", THETA, "DURATION", DURATION);
    }

    friend std::ostream &operator<<(std::ostream &os, const CommonAppConfig &config) {
        ConfigPrinter<CommonAppConfig>::print(os, config);
        return os;
    };
};

struct SequentialAppConfig : public CommonAppConfig {
    friend std::ostream &operator<<(std::ostream &os, const SequentialAppConfig &config) {
        ConfigPrinter<SequentialAppConfig>::print(os, config);
        return os;
    }
};

struct ParallelAppConfig : public CommonAppConfig {
    int NUM_THREADS;

    static void add_params_to_config_parser(ParallelAppConfig &parallel_app_config, ConfigParser &parser) {
        CommonAppConfig::add_params_to_config_parser(parallel_app_config, parser);
        parser.AddParameter(new IntParameter("app.num_threads", "20", &parallel_app_config.NUM_THREADS, false, "Number of threads for the parallel application"));
    }

    auto to_tuple() const {
        auto common_tuple = CommonAppConfig::to_tuple();
        return std::tuple_cat(common_tuple, std::make_tuple("NUM_THREADS", NUM_THREADS));
    }

    friend std::ostream &operator<<(std::ostream &os, const ParallelAppConfig &config) {
        ConfigPrinter<ParallelAppConfig>::print(os, config);
        return os;
    };
};