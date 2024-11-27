#pragma once

#include "utils/ConfigParser.hpp"
#include "utils/ConfigPrinter.hpp"
#include <getopt.h>
#include <string>

using std::string;

struct PRIFConfig {
    float BETA;
    float THETA;
    int NUM_THREADS;
    double QUERY_RATE;

    static void add_params_to_config_parser(PRIFConfig &prif_configs, ConfigParser &parser) {
        parser.AddParameter(new FloatParameter("prif.beta", "0.001", &prif_configs.BETA, false, "Beta value for the PRIF algorithm"));
        parser.AddParameter(new FloatParameter("prif.theta", "0.01", &prif_configs.THETA, false, "Theta value for the PRIF algorithm"));
        parser.AddParameter(new IntParameter("prif.num_threads", "20", &prif_configs.NUM_THREADS, false, "Number of threads for the PRIF algorithm"));
        parser.AddParameter(new DoubleParameter("prif.query_rate", "0", &prif_configs.QUERY_RATE, false, "Query rate for the PRIF algorithm"));
    }

    auto to_tuple() const { return std::make_tuple("BETA", BETA, "THETA", THETA, "NUM_THREADS", NUM_THREADS, "QUERY_RATE", QUERY_RATE); }

    friend std::ostream &operator<<(std::ostream &os, const PRIFConfig &config) {
        ConfigPrinter<PRIFConfig>::print(os, config);
        return os;
    }
};
