#include "example_support.hpp"

#include <chrono>
#include <cstdlib>
#include <thread>

int main(int argc, char** argv) {
    try {
        std::string port;
        std::optional<qnbot::Side> side;
        std::string package_id;
        std::string operator_id = "default";
        auto bone_usage = qnbot::BoneConfigUsage::auto_detect;
        std::string bone_option = "auto";
        std::optional<std::string> algorithm_config_path;
        double seconds = 10.0;
        for (int index = 1; index < argc; ++index) {
            const std::string argument = argv[index];
            const auto value =
                example::require_value(argc, argv, index, argument);
            if (argument == "--port")
                port = value;
            else if (argument == "--side")
                side = example::parse_side(value);
            else if (argument == "--package-id")
                package_id = value;
            else if (argument == "--operator-id")
                operator_id = value;
            else if (argument == "--algorithm-config-path")
                algorithm_config_path = value;
            else if (argument == "--seconds")
                seconds = example::parse_positive_double(value, argument);
            else if (argument == "--use-bone-config") {
                bone_option = value;
                if (value == "auto")
                    bone_usage = qnbot::BoneConfigUsage::auto_detect;
                else if (value == "true")
                    bone_usage = qnbot::BoneConfigUsage::required;
                else if (value == "false")
                    bone_usage = qnbot::BoneConfigUsage::disabled;
                else
                    throw std::invalid_argument(
                        "--use-bone-config must be auto, true or false");
            } else
                throw std::invalid_argument("unknown argument: " + argument);
        }
        if (port.empty()) throw std::invalid_argument("--port is required");
        if (!side) throw std::invalid_argument("--side is required");
        if (package_id.empty())
            throw std::invalid_argument("--package-id is required");
        auto config = example::serial_config(port, *side);
        config.algorithms.capture.operator_id = operator_id;
        qnbot::TargetConfig target;
        target.name = "configured_hand";
        target.side = *side;
        target.source = qnbot::DeviceSelector{"glove", std::nullopt, *side};
        target.algorithms = {
            qnbot::TargetAlgorithm{
                package_id,
                bone_usage,
                algorithm_config_path,
            },
        };
        config.targets.push_back(target);
        qnbot::Sdk sdk(config);
        auto glove = sdk.glove();
        auto device = glove.device();
        const auto subscription =
            device.output("configured_hand").subscribe(example::print_output);
        try {
            glove.start();
            glove.run_background();
            std::cout << "package=" << package_id
                      << " use_bone_config=" << bone_option
                      << " algorithm_config_path="
                      << algorithm_config_path.value_or("(default)")
                      << std::endl;
            std::this_thread::sleep_for(std::chrono::duration<double>(seconds));
            glove.request_stop();
            glove.join();
        } catch (...) {
            glove.close();
            throw;
        }
        static_cast<void>(subscription);
        glove.close();
        std::cout << "algorithm configuration complete\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        return example::report_error(error);
    }
}
