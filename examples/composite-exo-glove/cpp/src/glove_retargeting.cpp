#include "composite_example_support.hpp"

#include <cstdlib>
#include <iostream>
#include <optional>
#include <string>

namespace {

struct Options {
    std::string port;
    std::string package_id;
};

Options parse_options(int argc, char** argv) {
    Options options;
    for (int index = 1; index < argc; ++index) {
        const std::string argument = argv[index];
        if (argument == "--port") {
            options.port = example::require_value(argc, argv, index, argument);
        } else if (argument == "--package-id") {
            options.package_id =
                example::require_value(argc, argv, index, argument);
        } else {
            throw std::invalid_argument("unknown argument: " + argument);
        }
    }
    if (options.port.empty()) {
        throw std::invalid_argument("--port is required");
    }
    if (options.package_id.empty()) {
        throw std::invalid_argument("--package-id is required");
    }
    return options;
}

qnbot::SdkConfig make_config(const Options& options) {
    qnbot::SdkConfig config = example::composite_config(options.port);

    qnbot::TargetConfig left_target;
    left_target.side = qnbot::Side::left;
    left_target.source =
        qnbot::DeviceSelector{"glove", std::nullopt, qnbot::Side::left};
    left_target.algorithms = {qnbot::TargetAlgorithm{options.package_id}};

    qnbot::TargetConfig right_target;
    right_target.side = qnbot::Side::right;
    right_target.source =
        qnbot::DeviceSelector{"glove", std::nullopt, qnbot::Side::right};
    right_target.algorithms = {qnbot::TargetAlgorithm{options.package_id}};

    config.targets = {left_target, right_target};
    return config;
}

void print_output(const char* label,
                  const qnbot::Sample<qnbot::HandJointCommand>& sample) {
    std::cout << label << ' ';
    example::print_output(sample);
}

} // namespace

int main(int argc, char** argv) {
    try {
        const auto options = parse_options(argc, argv);
        qnbot::Sdk sdk(make_config(options));
        auto glove = sdk.glove();
        auto left = glove.device(qnbot::Side::left);
        auto right = glove.device(qnbot::Side::right);
        const auto left_output_subscription = left.output().subscribe(
            [](const qnbot::Sample<qnbot::HandJointCommand>& sample) {
                print_output("left", sample);
            });
        const auto right_output_subscription = right.output().subscribe(
            [](const qnbot::Sample<qnbot::HandJointCommand>& sample) {
                print_output("right", sample);
            });

        sdk.start();
        std::cout << "reading Glove retargeting output; "
                     "press Ctrl+C to stop\n";
        sdk.run_forever();
        sdk.stop();
        sdk.close();
        static_cast<void>(left_output_subscription);
        static_cast<void>(right_output_subscription);
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        return example::report_error(error);
    }
}
