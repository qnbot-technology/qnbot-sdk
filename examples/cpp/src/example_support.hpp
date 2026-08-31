#ifndef QNBOT_CPP_EXAMPLE_SUPPORT_HPP
#define QNBOT_CPP_EXAMPLE_SUPPORT_HPP

#include <qnbot/glove.hpp>

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>

namespace example {

constexpr double max_duration_seconds = 3600.0;
constexpr std::uint64_t max_update_count = 1000000;
constexpr const char* default_target_name = "openxr_hand";

struct SerialOptions {
    std::string port;
    std::optional<qnbot::Side> side;
    double seconds{1.0};
    std::uint64_t updates{10};
    double hold{1.0};
    std::string target_name{default_target_name};
    std::string package_id;
    bool package_id_explicit{false};
    bool validate_only{false};
};

enum class SerialExample {
    manual,
    background,
    foreground,
    haptics,
};

inline void validate_no_options(int argc, char** argv) {
    if (argc == 1) return;
    if (argc > 1) {
        throw std::invalid_argument("unknown argument: " +
                                    std::string(argv[1]));
    }
    throw std::invalid_argument("invalid argument count");
}

inline std::string require_value(int argc, char** argv, int& index,
                                 const std::string& option) {
    if (++index >= argc)
        throw std::invalid_argument(option + " requires a value");
    const std::string value = argv[index];
    if (value.rfind("--", 0) == 0) {
        throw std::invalid_argument(option + " requires a value");
    }
    return value;
}

inline double parse_positive_double(const std::string& text,
                                    const std::string& option,
                                    bool allow_zero = false) {
    if (text.empty() || text.front() == '-') {
        throw std::invalid_argument(option + " has an invalid value");
    }
    std::size_t consumed = 0;
    double value = 0.0;
    try {
        value = std::stod(text, &consumed);
    } catch (const std::invalid_argument&) {
        throw std::invalid_argument(option + " has an invalid value");
    } catch (const std::out_of_range&) {
        throw std::invalid_argument(option + " has an invalid value");
    }
    if (consumed != text.size() || !std::isfinite(value) ||
        (allow_zero ? value < 0.0 : value <= 0.0) ||
        value > max_duration_seconds) {
        throw std::invalid_argument(option + " has an invalid value");
    }
    return value;
}

inline std::uint64_t parse_positive_count(const std::string& text,
                                          const std::string& option) {
    if (text.empty()) {
        throw std::invalid_argument(option + " has an invalid value");
    }
    std::uint64_t value = 0;
    for (const char character : text) {
        if (character < '0' || character > '9') {
            throw std::invalid_argument(option + " has an invalid value");
        }
        const auto digit = static_cast<std::uint64_t>(character - '0');
        if (value > (max_update_count - digit) / 10) {
            throw std::invalid_argument(option + " has an invalid value");
        }
        value = value * 10 + digit;
    }
    if (value == 0 || value > max_update_count) {
        throw std::invalid_argument(option + " has an invalid value");
    }
    return value;
}

inline qnbot::Side parse_side(const std::string& text) {
    if (text == "left") return qnbot::Side::left;
    if (text == "right") return qnbot::Side::right;
    throw std::invalid_argument("--side must be left or right");
}

inline SerialOptions parse_serial_options(int argc, char** argv,
                                          SerialExample example) {
    SerialOptions options;
    for (int index = 1; index < argc; ++index) {
        const std::string argument = argv[index];
        if (argument == "--port") {
            options.port = require_value(argc, argv, index, argument);
        } else if (argument == "--side") {
            options.side =
                parse_side(require_value(argc, argv, index, argument));
        } else if (argument == "--seconds" &&
                   example == SerialExample::background) {
            options.seconds = parse_positive_double(
                require_value(argc, argv, index, argument), argument);
        } else if (argument == "--updates" &&
                   example == SerialExample::manual) {
            options.updates = parse_positive_count(
                require_value(argc, argv, index, argument), argument);
        } else if (argument == "--hold" && example == SerialExample::haptics) {
            options.hold = parse_positive_double(
                require_value(argc, argv, index, argument), argument, true);
        } else if (argument == "--target-name" &&
                   example != SerialExample::haptics) {
            options.target_name = require_value(argc, argv, index, argument);
        } else if (argument == "--package-id" &&
                   example != SerialExample::haptics) {
            options.package_id = require_value(argc, argv, index, argument);
            options.package_id_explicit = true;
        } else if (argument == "--validate") {
            options.validate_only = true;
        } else {
            throw std::invalid_argument("unknown argument: " + argument);
        }
    }
    if (options.port.empty()) {
        throw std::invalid_argument("--port is required");
    }
    if (!options.side) {
        throw std::invalid_argument("--side is required with --port");
    }
    if (example != SerialExample::haptics && !options.package_id_explicit) {
        throw std::invalid_argument("--package-id is required");
    }
    return options;
}

inline qnbot::SdkConfig serial_config(const std::string& port,
                                      qnbot::Side side) {
    qnbot::SerialConnection connection;
    connection.port = port;

    qnbot::GloveConfig glove{side, connection};
    glove.name = "primary";

    qnbot::SdkConfig config;
    config.devices.push_back(glove);
    return config;
}

inline qnbot::SdkConfig runtime_config(const SerialOptions& options) {
    const auto side = *options.side;
    auto config = serial_config(options.port, side);

    qnbot::TargetConfig target;
    target.type = qnbot::TargetType::hand;
    target.name = options.target_name;
    target.side = side;
    target.source =
        qnbot::DeviceSelector{"glove", std::string("primary"), std::nullopt};
    if (options.package_id_explicit) {
        target.algorithms = {qnbot::TargetAlgorithm{options.package_id}};
    }
    config.targets.push_back(std::move(target));
    return config;
}

inline void print_output(const qnbot::Sample<qnbot::HandJointCommand>& sample) {
    std::cout << "retargeting sequence=" << sample.sequence
              << " target=" << sample.value.target << " joints={";
    bool first = true;
    for (const auto& joint : sample.value.joints) {
        if (!first) std::cout << ", ";
        std::cout << joint.first << ": " << joint.second;
        first = false;
    }
    std::cout << "}\n";
}

inline void print_health(const std::string& label,
                         const qnbot::Health& health) {
    std::cout << label << " health ok=" << health.ok
              << " warnings=" << health.warning_count
              << " errors=" << health.error_count << " devices=[";
    bool first = true;
    for (const auto& entry : health.devices) {
        if (!first) std::cout << ", ";
        const auto& source_id = entry.first;
        const auto& device = entry.second;
        std::cout << source_id << "(connected=" << device.connected
                  << ", stale=" << device.stale << ')';
        first = false;
    }
    std::cout << "]\n";
}

inline void print_health(const qnbot::Health& health) {
    std::cout << "health ok=" << health.ok
              << " warnings=" << health.warning_count
              << " errors=" << health.error_count << '\n';
}

inline int report_error(const std::exception& error) {
    std::cerr << "qnbot example failed: " << error.what() << '\n';
    return EXIT_FAILURE;
}

} // namespace example

#endif
