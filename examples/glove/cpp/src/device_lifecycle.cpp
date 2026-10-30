#include <qnbot/glove.hpp>

#include <chrono>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>

namespace {

struct Options {
    std::optional<std::string> left_port;
    std::optional<std::string> right_port;
    std::uint64_t updates{10};
};

Options parse_options(int argc, char** argv) {
    Options options;
    for (int index = 1; index < argc; ++index) {
        const std::string argument = argv[index];
        if (argument == "--help") {
            std::cout << "usage: device_lifecycle [--left-port PORT] "
                         "[--right-port PORT] [--updates COUNT]\n";
            std::exit(EXIT_SUCCESS);
        }
        if (argument != "--left-port" && argument != "--right-port" &&
            argument != "--updates") {
            throw std::invalid_argument("unknown argument: " + argument);
        }
        if (++index >= argc) {
            throw std::invalid_argument(argument + " requires a value");
        }
        const std::string value = argv[index];
        if (argument == "--left-port") {
            options.left_port = value;
        } else if (argument == "--right-port") {
            options.right_port = value;
        } else {
            if (value.empty() ||
                value.find_first_not_of("0123456789") != std::string::npos) {
                throw std::invalid_argument(
                    "--updates must be a positive integer");
            }
            options.updates = std::stoull(value);
            if (options.updates == 0) {
                throw std::invalid_argument("--updates must be at least 1");
            }
        }
    }
    return options;
}

void print_pose(const char* name,
                const qnbot::Sample<qnbot::GlovePose>& sample) {
    std::cout << name << " pose sequence=" << sample.sequence
              << " fingertips=" << sample.value.payload.fingertip_local.size()
              << '\n';
}

} // namespace

int main(int argc, char** argv) {
    try {
        const auto options = parse_options(argc, argv);
        qnbot::GloveConfig left_config;
        left_config.name = "left";
        left_config.side = qnbot::Side::left;
        qnbot::SerialConnection left_connection;
        left_connection.port = options.left_port;
        left_config.connection = left_connection;
        qnbot::GloveConfig right_config;
        right_config.name = "right";
        right_config.side = qnbot::Side::right;
        qnbot::SerialConnection right_connection;
        right_connection.port = options.right_port;
        right_config.connection = right_connection;
        qnbot::SdkConfig config;
        config.devices = {left_config, right_config};
        qnbot::Sdk sdk(std::move(config));
        auto glove = sdk.glove();
        try {
            auto left = glove.device("left");
            auto right = glove.device("right");
            auto left_pose = left.pose();
            auto right_pose = right.pose();
            const auto observe = [&](bool right_active) {
                auto left_previous = left_pose.latest();
                auto right_previous = right_pose.latest();
                std::optional<std::uint64_t> left_sequence;
                std::optional<std::uint64_t> right_sequence;
                if (left_previous) left_sequence = left_previous->sequence;
                if (right_previous) right_sequence = right_previous->sequence;
                std::uint64_t left_count = 0;
                std::uint64_t right_count = 0;
                const auto deadline =
                    std::chrono::steady_clock::now() + std::chrono::seconds(10);
                while (left_count < options.updates ||
                       (right_active && right_count < options.updates)) {
                    if (std::chrono::steady_clock::now() >= deadline) {
                        throw std::runtime_error(
                            "timed out waiting for pose samples");
                    }
                    const auto update = glove.update();
                    const auto left_sample = left_pose.latest();
                    const auto right_sample = right_pose.latest();
                    const auto current_right =
                        right_sample ? std::optional<std::uint64_t>(
                                           right_sample->sequence)
                                     : std::nullopt;
                    if (!right_active && current_right != right_sequence) {
                        throw std::runtime_error(
                            "right pose advanced while stopped");
                    }
                    if (left_sample && left_sample->sequence != left_sequence) {
                        left_sequence = left_sample->sequence;
                        ++left_count;
                        print_pose("left", *left_sample);
                    }
                    if (right_active && right_sample &&
                        right_sample->sequence != right_sequence) {
                        right_sequence = right_sample->sequence;
                        ++right_count;
                        print_pose("right", *right_sample);
                    }
                    if (update.sleep() == qnbot::SleepResult::no_tasks) {
                        std::this_thread::sleep_for(
                            std::chrono::milliseconds(10));
                    }
                }
            };

            left.start();
            right.start();
            std::cout << "both devices active\n";
            observe(true);
            right.stop();
            std::cout << "right device stopped; left remains active\n";
            observe(false);
            right.start();
            std::cout << "right device restarted\n";
            observe(true);
            std::cout << "lifecycle complete\n";
        } catch (...) {
            glove.close();
            throw;
        }
        glove.close();
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "device lifecycle failed: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
