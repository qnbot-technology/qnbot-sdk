#include "example_support.hpp"

#include <cstdlib>
#include <iostream>
#include <utility>
#include <vector>

namespace {

std::string parse_package_id(int argc, char** argv) {
    std::string package_id;
    for (int index = 1; index < argc; ++index) {
        const std::string argument = argv[index];
        if (argument == "--package-id") {
            package_id = example::require_value(argc, argv, index, argument);
        } else {
            throw std::invalid_argument("unknown argument: " + argument);
        }
    }
    if (package_id.empty()) {
        throw std::invalid_argument("--package-id is required");
    }
    return package_id;
}

} // namespace

int main(int argc, char** argv) {
    try {
        const auto package_id = parse_package_id(argc, argv);
        qnbot::GloveConfig left;
        left.side = qnbot::Side::left;
        qnbot::GloveConfig right;
        right.side = qnbot::Side::right;
        qnbot::GloveReceiverConfig receiver;
        receiver.devices = {left, right};

        qnbot::SdkConfig config;
        config.devices = {receiver};
        for (const auto side : {qnbot::Side::left, qnbot::Side::right}) {
            qnbot::TargetConfig target;
            target.side = side;
            target.source = qnbot::DeviceSelector{"glove", std::nullopt, side};
            target.algorithms = {qnbot::TargetAlgorithm{package_id}};
            config.targets.push_back(target);
        }
        qnbot::Sdk sdk(std::move(config));
        auto glove = sdk.glove();

        std::vector<qnbot::Subscription> subscriptions;
        for (const auto side : {qnbot::Side::left, qnbot::Side::right}) {
            const auto label = side == qnbot::Side::left ? "left" : "right";
            auto device = glove.device(side);
            subscriptions.push_back(device.output().subscribe(
                [label](const qnbot::Sample<qnbot::HandJointCommand>& sample) {
                    std::cout << label << ' ';
                    example::print_output(sample);
                }));
        }

        try {
            glove.start();
            std::cout << "running; press Ctrl+C to stop" << std::endl;
            glove.run_forever();
        } catch (...) {
            glove.close();
            throw;
        }
        glove.close();
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        return example::report_error(error);
    }
}
