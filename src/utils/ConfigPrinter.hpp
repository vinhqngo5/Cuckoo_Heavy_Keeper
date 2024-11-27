#pragma once
#include <algorithm>
#include <cxxabi.h>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <tuple>
#include <type_traits>

template <typename T> class ConfigPrinter {
  private:
    static constexpr size_t LABEL_WIDTH = 32;
    static constexpr size_t PADDING = 4;

    template <typename U> static std::string to_string_with_precision(const U &value, int precision = 6) {
        std::ostringstream out;
        out << std::fixed << std::setprecision(precision) << value;
        return out.str();
    }

    template <typename U> static std::string value_to_string(const U &value) {
        if constexpr (std::is_same_v<U, float> || std::is_same_v<U, double>) {
            return to_string_with_precision(value);
        } else if constexpr (std::is_same_v<U, std::string>) {
            return value;
        } else {
            return std::to_string(value);
        }
    }

    template <typename Tuple, size_t... Is> static void print_fields(std::ostream &os, const Tuple &t, size_t box_width, std::index_sequence<Is...>) {
        (print_field(os, std::get<Is * 2>(t), std::get<Is * 2 + 1>(t), box_width), ...);
    }

    template <typename U> static void print_field(std::ostream &os, const std::string &name, const U &value, size_t box_width) {
        std::string value_str = value_to_string(value);
        os << "| " << std::left << std::setw(LABEL_WIDTH) << name << ": " << std::left << std::setw(box_width - LABEL_WIDTH - PADDING) << value_str << "|" << std::endl;
    }

    template <typename Tuple, size_t... Is> static size_t calculate_box_width(const Tuple &t, std::index_sequence<Is...>) {
        return std::max({(LABEL_WIDTH + value_to_string(std::get<Is * 2 + 1>(t)).length() + PADDING)...});
    }

  public:
    static std::string demangle(const char *name) {
        int status = 0;
        std::unique_ptr<char, void (*)(void *)> res{abi::__cxa_demangle(name, NULL, NULL, &status), std::free};
        return (status == 0) ? res.get() : name;
    }

    static void print(std::ostream &os, const T &config) {
        std::string class_name = demangle(typeid(T).name());
        auto tuple = config.to_tuple();
        constexpr size_t num_fields = std::tuple_size_v<decltype(tuple)> / 2;

        size_t box_width = std::max({calculate_box_width(tuple, std::make_index_sequence<num_fields>{}), class_name.length() + PADDING, LABEL_WIDTH + PADDING});

        std::string horizontal_line("+" + std::string(box_width - 1, '-') + "+");

        os << horizontal_line << std::endl;
        os << "| " << std::left << std::setw(box_width - 2) << class_name << "|" << std::endl;
        os << horizontal_line << std::endl;

        print_fields(os, tuple, box_width, std::make_index_sequence<num_fields>{});

        os << horizontal_line << std::endl;
        os << std::endl;   // Add an extra line for spacing
    }
};