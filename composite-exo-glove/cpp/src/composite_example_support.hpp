#ifndef QNBOT_CPP_COMPOSITE_EXO_GLOVE_EXAMPLE_SUPPORT_HPP
#define QNBOT_CPP_COMPOSITE_EXO_GLOVE_EXAMPLE_SUPPORT_HPP

#include <qnbot/exo.hpp>
#include <qnbot/glove.hpp>
#include <qnbot/sdk.hpp>

#include <array>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

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

inline std::string parse_port(int argc, char** argv) {
    std::string port;
    for (int index = 1; index < argc; ++index) {
        const std::string argument = argv[index];
        if (argument == "--port") {
            port = require_value(argc, argv, index, argument);
        } else {
            throw std::invalid_argument("unknown argument: " + argument);
        }
    }
    if (port.empty()) throw std::invalid_argument("--port is required");
    return port;
}

inline qnbot::SdkConfig
composite_config(std::optional<std::string> port = std::nullopt) {
    qnbot::GloveConfig glove;
    glove.name = "glove";
    glove.side = qnbot::Side::right;

    qnbot::ExoConfig exo;
    exo.name = "exo";

    qnbot::CompositeExoGloveConfig composite;
    if (port) {
        composite.connection.port = std::move(*port);
    } else {
        composite.connection.auto_discover = true;
    }
    composite.devices = {qnbot::DeviceConfig{glove}, qnbot::DeviceConfig{exo}};

    qnbot::SdkConfig config;
    config.devices.push_back(composite);
    return config;
}

inline void print_glove_info(const qnbot::GloveDeviceInfo& info) {
    std::cout << "glove sn=" << info.canonical_sn
              << " type=" << info.device_type << " model=" << info.model
              << " firmware=" << info.firmware_version
              << " hand=" << (info.hand == qnbot::Side::left ? "left" : "right")
              << '\n';
}

inline void print_exo_info(const qnbot::ExoDeviceInfo& info) {
    std::cout << "exo product=" << static_cast<int>(info.product)
              << " protocol=" << static_cast<int>(info.protocol_version.major)
              << '.' << static_cast<int>(info.protocol_version.minor);
    if (info.serial_number) std::cout << " sn=" << *info.serial_number;
    if (info.hardware_name) std::cout << " name=" << *info.hardware_name;
    std::cout << " capabilities(telemetry=" << info.capabilities.telemetry
              << ", imus=" << info.capabilities.imus
              << ", haptics=" << info.capabilities.haptics << ")\n";
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
    std::cerr << "qnbot composite example failed: " << error.what() << '\n';
    return EXIT_FAILURE;
}

} // namespace example

#endif
