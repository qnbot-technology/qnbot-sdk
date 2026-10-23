#ifndef QNBOT_PUBLIC_EXAMPLE_EXTERNAL_INPUT_SUPPORT_HPP
#define QNBOT_PUBLIC_EXAMPLE_EXTERNAL_INPUT_SUPPORT_HPP

#include "example_support.hpp"

#include <qnbot/glove.hpp>

#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>

namespace external_input_example {

inline qnbot::GloveNodePose node(double x, double y, double z) {
    return {{x, y, z}, {0.0, 0.0, 0.0, 1.0}};
}

inline qnbot::ExternalGloveFrame frame(std::uint64_t step) {
    const double offset = static_cast<double>(step) * 0.001;
    qnbot::ExternalGloveFrame result;
    result.thumb = node(0.01 + offset, 0.02, 0.03);
    result.index = node(0.02 + offset, 0.03, 0.04);
    result.middle = node(0.03 + offset, 0.04, 0.05);
    result.ring = node(0.04 + offset, 0.05, 0.06);
    result.pinky = node(0.05 + offset, 0.06, 0.07);
    result.timestamp_ms = static_cast<double>(step) * 10.0;
    return result;
}

struct Options {
    std::string package_id;
    qnbot::CaptureInteractionMode interaction =
        qnbot::CaptureInteractionMode::auto_terminal;
};

inline Options parse_options(int argc, char** argv) {
    Options options;
    std::optional<std::string> package_id;
    for (int index = 1; index < argc; ++index) {
        const std::string argument = argv[index];
        if (argument == "--package-id") {
            if (package_id)
                throw std::invalid_argument(
                    "--package-id may be provided once");
            package_id = example::require_value(argc, argv, index, argument);
        } else if (argument == "--capture-interaction") {
            const auto value =
                example::require_value(argc, argv, index, argument);
            if (value == "auto_terminal") {
                options.interaction =
                    qnbot::CaptureInteractionMode::auto_terminal;
            } else if (value == "external") {
                options.interaction = qnbot::CaptureInteractionMode::external;
            } else {
                throw std::invalid_argument(
                    "--capture-interaction must be auto_terminal or external");
            }
        } else {
            throw std::invalid_argument("unknown argument: " + argument);
        }
    }
    if (!package_id) throw std::invalid_argument("--package-id is required");
    options.package_id = *package_id;
    return options;
}

inline qnbot::SdkConfig make_config(const Options& options) {
    qnbot::GloveConfig glove{
        std::string{"external"},
        qnbot::Side::right,
        qnbot::ExternalConnection{},
        true,
    };

    qnbot::TargetConfig target;
    target.source =
        qnbot::DeviceSelector{"glove", std::nullopt, qnbot::Side::right};
    target.name = "hand";
    target.side = qnbot::Side::right;
    target.algorithms = {qnbot::TargetAlgorithm{options.package_id}};

    qnbot::SdkConfig config;
    config.devices.push_back(glove);
    config.targets.push_back(target);
    config.algorithms.capture.interaction = options.interaction;
    return config;
}

} // namespace external_input_example

#endif
