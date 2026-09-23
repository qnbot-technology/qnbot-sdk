#ifndef QNBOT_CPP_EXO_EXAMPLE_SUPPORT_HPP
#define QNBOT_CPP_EXO_EXAMPLE_SUPPORT_HPP

#include <qnbot/exo.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>

namespace example {

inline std::string require_value(int argc, char** argv, int& index,
                                 const std::string& option) {
    if (++index >= argc) {
        throw std::invalid_argument(option + " requires a value");
    }
    const std::string value = argv[index];
    if (value.rfind("--", 0) == 0) {
        throw std::invalid_argument(option + " requires a value");
    }
    return value;
}

inline double parse_seconds(const std::string& text) {
    std::size_t consumed = 0;
    double value = 0.0;
    try {
        value = std::stod(text, &consumed);
    } catch (const std::exception&) {
        throw std::invalid_argument("--seconds has an invalid value");
    }
    if (consumed != text.size() || value < 0.0) {
        throw std::invalid_argument("--seconds has an invalid value");
    }
    return value;
}

inline std::uint8_t parse_strength(const std::string& text,
                                   const std::string& option) {
    std::size_t consumed = 0;
    long value = 0;
    try {
        value = std::stol(text, &consumed);
    } catch (const std::exception&) {
        throw std::invalid_argument(option + " has an invalid value");
    }
    if (consumed != text.size() || value < 0 || value > 100) {
        throw std::invalid_argument(option + " must be in 0..100");
    }
    return static_cast<std::uint8_t>(value);
}

inline qnbot::SdkConfig serial_config(const std::string& port,
                                      const std::string& name) {
    qnbot::ExoConfig exo;
    exo.connection.port = port;
    exo.name = name;

    qnbot::SdkConfig config;
    config.devices.push_back(exo);
    return config;
}

inline void print_health(const std::string& label,
                         const qnbot::Health& health) {
    std::cout << label << " health ok=" << health.ok
              << " warnings=" << health.warning_count
              << " errors=" << health.error_count << '\n';
}

inline void print_encoder_counts(const std::array<std::int16_t, 8>& counts) {
    std::cout << '[';
    for (std::size_t index = 0; index < counts.size(); ++index) {
        if (index != 0) std::cout << ", ";
        std::cout << counts[index];
    }
    std::cout << ']';
}

template <std::size_t N>
inline void print_values(const std::array<double, N>& values) {
    std::cout << '[';
    for (std::size_t index = 0; index < values.size(); ++index) {
        if (index != 0) std::cout << ", ";
        std::cout << values[index];
    }
    std::cout << ']';
}

inline int report_error(const std::exception& error) {
    std::cerr << "qnbot example failed: " << error.what() << '\n';
    return EXIT_FAILURE;
}

} // namespace example

#endif
