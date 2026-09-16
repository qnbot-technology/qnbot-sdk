#include "example_support.hpp"

#include <cstdlib>
#include <iostream>
#include <optional>
#include <string>

namespace {

struct Options {
    std::string port;
    std::optional<qnbot::Side> side;
    std::string package_id;
};

Options parse_options(int argc, char** argv) {
    Options options;
    for (int index = 1; index < argc; ++index) {
        const std::string argument = argv[index];
        if (argument == "--port") {
            options.port = example::require_value(argc, argv, index, argument);
        } else if (argument == "--side") {
            options.side = example::parse_side(
                example::require_value(argc, argv, index, argument));
        } else if (argument == "--package-id") {
            options.package_id =
                example::require_value(argc, argv, index, argument);
        } else {
            throw std::invalid_argument("unknown argument: " + argument);
        }
    }
    if (options.port.empty()) throw std::invalid_argument("--port is required");
    if (!options.side) throw std::invalid_argument("--side is required");
    if (options.package_id.empty()) {
        throw std::invalid_argument("--package-id is required");
    }
    return options;
}

qnbot::SdkConfig make_config(const Options& options) {
    auto config = example::serial_config(options.port, *options.side);
    qnbot::TargetConfig target;
    target.name = "primary";
    target.side = options.side;
    target.source =
        qnbot::DeviceSelector{"glove", std::string("primary"), std::nullopt};
    target.algorithms = {qnbot::TargetAlgorithm{options.package_id}};
    config.targets.push_back(target);
    target.name = "backup";
    config.targets.push_back(std::move(target));
    return config;
}

} // namespace

int main(int argc, char** argv) {
    try {
        const auto options = parse_options(argc, argv);
        qnbot::Sdk sdk(make_config(options));
        auto glove = sdk.glove();

        auto device = glove.device("primary");
        auto primary = device.output("primary");
        auto backup = device.output("backup");
        const auto primary_subscription = primary.subscribe(
            [](const qnbot::Sample<qnbot::HandJointCommand>& sample) {
                std::cout << "primary ";
                example::print_output(sample);
            });
        const auto backup_subscription = backup.subscribe(
            [](const qnbot::Sample<qnbot::HandJointCommand>& sample) {
                std::cout << "backup ";
                example::print_output(sample);
            });

        glove.start();
        std::cout << "running; press Ctrl+C to stop" << std::endl;
        glove.run_forever();
        static_cast<void>(primary_subscription);
        static_cast<void>(backup_subscription);
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        return example::report_error(error);
    }
}
