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
    std::string package_id;
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
        } else if (argument == "--package-id") {
            options.package_id =
                example::require_value(argc, argv, index, argument);
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
    if (options.package_id.empty()) {
        throw std::invalid_argument("--package-id is required");
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
    left_target.name = options.target_name;
    left_target.side = qnbot::Side::left;
    left_target.source =
        qnbot::DeviceSelector{"glove", std::string("left"), std::nullopt};
    left_target.algorithms = {qnbot::TargetAlgorithm{options.package_id}};

    qnbot::TargetConfig right_target;
    right_target.name = options.target_name;
    right_target.side = qnbot::Side::right;
    right_target.source =
        qnbot::DeviceSelector{"glove", std::string("right"), std::nullopt};
    right_target.algorithms = {qnbot::TargetAlgorithm{options.package_id}};

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
        auto glove = sdk.glove();

        auto left = glove.device("left");
        auto right = glove.device(qnbot::Side::right);
        auto left_output = left.output(options.target_name);
        auto right_output = right.output(options.target_name);
        left.start();
        right.start();

        for (std::uint64_t index = 0; index < options.updates; ++index) {
            const auto update = glove.update();
            if (update.has_next_task()) static_cast<void>(update.sleep());
            if (const auto sample = left_output.latest()) {
                print_output("left", *sample);
            }
            if (const auto sample = right_output.latest()) {
                print_output("right", *sample);
            }
        }

        const auto left_before_stop = left_output.latest();
        if (!left_before_stop) {
            throw std::runtime_error(
                "both gloves must produce output before lifecycle control");
        }

        right.stop();
        std::cout << "right device stopped; left remains active\n";
        const auto right_stopped_baseline = right_output.latest();
        if (!right_stopped_baseline) {
            throw std::runtime_error("right output was unavailable after stop");
        }
        std::optional<qnbot::Sample<qnbot::HandJointCommand>> left_sample;
        for (std::uint64_t index = 0; index < options.updates; ++index) {
            const auto update = glove.update();
            if (update.has_next_task()) static_cast<void>(update.sleep());
            const auto right_candidate = right_output.latest();
            if (right_candidate &&
                right_candidate->sequence > right_stopped_baseline->sequence) {
                throw std::runtime_error(
                    "right output advanced while the device was stopped");
            }
            const auto candidate = left_output.latest();
            if (candidate && candidate->sequence > left_before_stop->sequence) {
                left_sample = candidate;
                break;
            }
        }
        if (!left_sample) {
            throw std::runtime_error(
                "left output did not advance while right was stopped");
        }
        std::cout << "left output while right stopped\n";
        print_output("left", *left_sample);

        const auto right_before_restart = right_output.latest();
        if (!right_before_restart) {
            throw std::runtime_error(
                "right output was unavailable before restart");
        }
        right.start();
        std::cout << "right device restarted\n";
        std::optional<qnbot::Sample<qnbot::HandJointCommand>> right_sample;
        for (std::uint64_t index = 0; index < options.updates; ++index) {
            const auto update = glove.update();
            if (update.has_next_task()) static_cast<void>(update.sleep());
            const auto candidate = right_output.latest();
            if (candidate &&
                candidate->sequence > right_before_restart->sequence) {
                right_sample = candidate;
                break;
            }
        }
        if (!right_sample) {
            throw std::runtime_error(
                "right output did not advance after restart");
        }
        std::cout << "right output after restart\n";
        print_output("right", *right_sample);
        example::print_health(glove.health());
        glove.close();
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        return example::report_error(error);
    }
}
