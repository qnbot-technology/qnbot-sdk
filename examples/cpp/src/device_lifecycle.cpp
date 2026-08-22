#include "example_cleanup.hpp"
#include "example_support.hpp"

#include <cstdlib>
#include <iostream>
#include <optional>
#include <string>

namespace {

struct Options {
    std::string left_port;
    std::string right_port;
    std::uint64_t updates{10};
    std::string target_name{example::default_target_name};
    std::string algorithm_id{example::default_algorithm_id};
    bool validate_only{false};
};

Options parse_options(int argc, char** argv) {
    Options options;
    for (int index = 1; index < argc; ++index) {
        const std::string argument = argv[index];
        if (argument == "--left-port") {
            options.left_port =
                example::require_value(argc, argv, index, argument);
        } else if (argument == "--right-port") {
            options.right_port =
                example::require_value(argc, argv, index, argument);
        } else if (argument == "--updates") {
            options.updates = example::parse_positive_count(
                example::require_value(argc, argv, index, argument), argument);
        } else if (argument == "--target-name") {
            options.target_name =
                example::require_value(argc, argv, index, argument);
        } else if (argument == "--algorithm-id") {
            options.algorithm_id =
                example::require_value(argc, argv, index, argument);
        } else if (argument == "--validate") {
            options.validate_only = true;
        } else {
            throw std::invalid_argument("unknown argument: " + argument);
        }
    }
    if (options.left_port.empty()) {
        throw std::invalid_argument("--left-port is required");
    }
    if (options.right_port.empty()) {
        throw std::invalid_argument("--right-port is required");
    }
    if (options.left_port == options.right_port) {
        throw std::invalid_argument(
            "--left-port and --right-port must be different");
    }
    return options;
}

qnbot::SdkConfig make_config(const Options& options) {
    qnbot::SerialConnection left_connection;
    left_connection.port = options.left_port;
    qnbot::GloveConfig left{qnbot::Side::left, left_connection};
    left.name = "left";

    qnbot::SerialConnection right_connection;
    right_connection.port = options.right_port;
    qnbot::GloveConfig right{qnbot::Side::right, right_connection};
    right.name = "right";

    qnbot::TargetConfig left_target;
    left_target.type = qnbot::TargetType::hand;
    left_target.name = options.target_name;
    left_target.side = qnbot::Side::left;
    left_target.source =
        qnbot::DeviceSelector{"glove", std::string("left"), std::nullopt};
    left_target.algorithms = {qnbot::TargetAlgorithm{options.algorithm_id}};

    qnbot::TargetConfig right_target;
    right_target.type = qnbot::TargetType::hand;
    right_target.name = options.target_name;
    right_target.side = qnbot::Side::right;
    right_target.source =
        qnbot::DeviceSelector{"glove", std::string("right"), std::nullopt};
    right_target.algorithms = {qnbot::TargetAlgorithm{options.algorithm_id}};

    qnbot::SdkConfig config;
    config.devices = {left, right};
    config.targets = {left_target, right_target};
    return config;
}

void print_output(const char* device_name,
                  const qnbot::Sample<qnbot::HandJointCommand>& sample) {
    std::cout << device_name << ' ';
    example::print_output(sample);
}

} // namespace

int main(int argc, char** argv) {
    try {
        const auto options = parse_options(argc, argv);
        qnbot::Sdk sdk(make_config(options));
        example::Cleanup cleanup;
        std::optional<qnbot::Glove> glove;
        try {
            glove.emplace(sdk.glove());
            if (!options.validate_only) {
                glove->connect();
                try {
                    static_cast<void>(glove->device());
                    throw std::runtime_error(
                        "unqualified device selection was not ambiguous");
                } catch (const qnbot::AmbiguousSelectionError&) {
                    std::cout << "selection requires name or side\n";
                }

                auto left = glove->device("left");
                auto right = glove->device(qnbot::Side::right);
                auto left_output = left.output(options.target_name);
                auto right_output = right.output(options.target_name);
                left.start();
                right.start();

                std::optional<std::uint64_t> left_sequence;
                std::optional<std::uint64_t> right_sequence;
                for (std::uint64_t index = 0; index < options.updates;
                     ++index) {
                    const auto update = glove->update();
                    if (update.has_next_task())
                        static_cast<void>(update.sleep());
                    if (const auto sample = left_output.latest()) {
                        if (sample->sequence != left_sequence) {
                            print_output("left", *sample);
                            left_sequence = sample->sequence;
                        }
                    }
                    if (const auto sample = right_output.latest()) {
                        if (sample->sequence != right_sequence) {
                            print_output("right", *sample);
                            right_sequence = sample->sequence;
                        }
                    }
                    if (left_sequence && right_sequence) break;
                }
                if (!left_sequence || !right_sequence) {
                    throw std::runtime_error(
                        "both gloves must produce output before disconnect");
                }

                right.stop();
                right.disconnect();
                std::cout << "right device disconnected\n";

                bool left_remains_usable = false;
                for (std::uint64_t index = 0; index < options.updates;
                     ++index) {
                    const auto update = glove->update();
                    if (update.has_next_task())
                        static_cast<void>(update.sleep());
                    if (const auto sample = left_output.latest()) {
                        if (sample->sequence != left_sequence) {
                            print_output("left after right disconnect",
                                         *sample);
                            left_remains_usable = true;
                            break;
                        }
                    }
                }
                if (!left_remains_usable) {
                    throw std::runtime_error(
                        "left glove stopped producing after right disconnect");
                }
                auto& domain = *glove;
                example::print_health(domain.health());
            }
        } catch (...) {
            cleanup.capture_current("device lifecycle");
        }

        if (glove) cleanup.run("glove.close()", [&] { glove->close(); });
        cleanup.run("sdk.close()", [&] { sdk.close(); });
        cleanup.rethrow_if_failed();
        if (options.validate_only) {
            std::cout << "device lifecycle configuration valid\n";
        }
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        return example::report_error(error);
    }
}
