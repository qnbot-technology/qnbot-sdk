#ifndef QNBOT_PUBLIC_EXAMPLE_EXTERNAL_INPUT_SUPPORT_HPP
#define QNBOT_PUBLIC_EXAMPLE_EXTERNAL_INPUT_SUPPORT_HPP

#include "example_support.hpp"

#include <qnbot/glove.hpp>

#include <cstdint>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_set>

namespace external_input_example {

inline qnbot::GloveNodePose node(double x, double y, double z) {
    return {{x, y, z}, {0.0, 0.0, 0.0, 1.0}};
}

inline qnbot::ExternalGloveFrame frame(std::uint64_t step) {
    const double offset = static_cast<double>(step) * 0.001;
    return {
        node(0.01 + offset, 0.02, 0.03), node(0.02 + offset, 0.03, 0.04),
        node(0.03 + offset, 0.04, 0.05), node(0.04 + offset, 0.05, 0.06),
        node(0.05 + offset, 0.06, 0.07), static_cast<double>(step) * 0.01,
    };
}

inline std::string parse_package_id(int argc, char** argv) {
    std::optional<std::string> package_id;
    for (int index = 1; index < argc; ++index) {
        const std::string argument = argv[index];
        if (argument == "--package-id") {
            if (package_id)
                throw std::invalid_argument("--package-id may be provided once");
            package_id =
                example::require_value(argc, argv, index, argument);
        } else {
            throw std::invalid_argument("unknown argument: " + argument);
        }
    }
    if (!package_id) throw std::invalid_argument("--package-id is required");
    return *package_id;
}

inline qnbot::SdkConfig make_config(const std::string& package_id) {
    qnbot::GloveConfig glove{qnbot::Side::right,
                             qnbot::ExternalConnection{}};
    glove.name = "external";

    qnbot::TargetConfig target{
        qnbot::TargetType::hand,
        qnbot::DeviceSelector{"glove", std::string("external"),
                              std::nullopt}};
    target.name = "hand";
    target.side = qnbot::Side::right;
    target.algorithms = {qnbot::TargetAlgorithm{package_id}};

    qnbot::SdkConfig config;
    config.devices.push_back(glove);
    config.targets.push_back(target);
    config.algorithms.calibration.interaction =
        qnbot::CalibrationInteractionMode::external;
    return config;
}

inline void confirm_calibration(
    const qnbot::ReadChannel<qnbot::CalibrationProgress>& progress,
    const qnbot::CalibrationControl& control,
    std::unordered_set<std::string>& confirmed_request_ids) {
    const auto sample = progress.latest();
    if (!sample) return;
    if (sample->value.state == qnbot::CalibrationProgressState::failed) {
        const std::string message = sample->value.failure
                                        ? sample->value.failure->message
                                        : "unknown error";
        throw std::runtime_error("external input calibration failed: " +
                                 message);
    }
    if (sample->value.state !=
            qnbot::CalibrationProgressState::awaiting_confirmation ||
        !sample->value.request_id ||
        confirmed_request_ids.count(*sample->value.request_id) != 0) {
        return;
    }

    std::cout << sample->value.prompt.value_or("Continue calibration")
              << " [Y/n]: ";
    std::string answer;
    std::getline(std::cin, answer);
    if (!answer.empty() && answer != "y" && answer != "Y" &&
        answer != "yes" && answer != "YES") {
        throw std::runtime_error("external input calibration was not confirmed");
    }
    control.confirm(*sample->value.request_id);
    confirmed_request_ids.insert(*sample->value.request_id);
}

} // namespace external_input_example

#endif

