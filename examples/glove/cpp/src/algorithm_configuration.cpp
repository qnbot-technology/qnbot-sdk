#include "example_support.hpp"

#include <chrono>
#include <cstdlib>
#include <thread>

namespace {
qnbot::BoneConfigUsage parse_bone_usage(const std::string& value) {
    if (value == "auto") return qnbot::BoneConfigUsage::auto_detect;
    if (value == "true") return qnbot::BoneConfigUsage::required;
    if (value == "false") return qnbot::BoneConfigUsage::disabled;
    throw std::invalid_argument(
        "--use-bone-config must be auto, true or false");
}
} // namespace

int main(int argc, char** argv) {
    try {
        std::string port;
        std::optional<qnbot::Side> side;
        std::string package_id;
        std::string operator_id = "default";
        std::optional<std::string> source_name;
        auto bone_usage = qnbot::BoneConfigUsage::auto_detect;
        std::string bone_option = "auto";
        std::optional<std::string> algorithm_config_path;
        std::optional<std::string> calibration_key;
        std::optional<std::string> target_name;
        std::optional<std::string> key_output;
        double seconds = 10.0;
        for (int index = 1; index < argc; ++index) {
            const auto [option, value] =
                example::next_argument(argc, argv, index);
            if (option == "--port")
                port = value;
            else if (option == "--side")
                side = example::parse_side(value);
            else if (option == "--package-id")
                package_id = value;
            else if (option == "--source-name")
                source_name = value;
            else if (option == "--operator-id")
                operator_id = value;
            else if (option == "--algorithm-config-path")
                algorithm_config_path = value;
            else if (option == "--calibration-key")
                calibration_key = value;
            else if (option == "--target-name")
                target_name = value;
            else if (option == "--key-output")
                key_output = value;
            else if (option == "--seconds")
                seconds = example::parse_positive_double(value, option);
            else if (option == "--use-bone-config") {
                bone_option = value;
                bone_usage = parse_bone_usage(value);
            } else
                throw std::invalid_argument("unknown argument: " + option);
        }
        if (!side) throw std::invalid_argument("--side is required");
        if (port.empty()) throw std::invalid_argument("--port is required");
        if (package_id.empty())
            throw std::invalid_argument("--package-id is required");

        qnbot::SerialConnection connection;
        connection.port = port;
        qnbot::GloveConfig configured_glove{*side, connection};
        configured_glove.name = source_name;
        qnbot::SdkConfig config;
        config.devices.push_back(configured_glove);
        config.algorithms.capture.operator_id = operator_id;
        qnbot::TargetConfig target;
        if (!calibration_key && !target_name) target_name = "configured_hand";
        target.name = target_name;
        target.side = *side;
        target.source = qnbot::DeviceSelector{"glove", source_name, *side};
        target.algorithms = {
            qnbot::TargetAlgorithm{
                package_id,
                bone_usage,
                algorithm_config_path,
                calibration_key,
            },
        };
        config.targets.push_back(target);
        qnbot::Sdk sdk(config);
        auto glove = sdk.glove();
        auto device = glove.device();
        const auto subscription =
            (target_name ? device.output(*target_name) : device.output())
                .subscribe(example::print_output);
        glove.start();
        glove.run_background();
        std::cout << "package=" << package_id
                  << " use_bone_config=" << bone_option
                  << " algorithm_config_path="
                  << algorithm_config_path.value_or("(default)") << std::endl;
        std::this_thread::sleep_for(std::chrono::duration<double>(seconds));
        if (key_output) {
            const auto saved =
                device.export_calibration(*key_output, target_name);
            std::cout << "effective calibration key saved: " << *key_output
                      << " sha256="
                      << saved.as_object().at("key_sha256").as_string() << '\n';
        }
        glove.request_stop();
        glove.join();
        static_cast<void>(subscription);
        glove.close();
        std::cout << "algorithm configuration complete\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        return example::report_error(error);
    }
}
