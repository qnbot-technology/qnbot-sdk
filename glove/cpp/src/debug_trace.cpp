#include "example_support.hpp"

#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <thread>

namespace {

static_assert(std::atomic<bool>::is_always_lock_free);
std::atomic<bool> interrupted{false};

void record_interruption(int) noexcept {
    interrupted.store(true, std::memory_order_relaxed);
}

struct Options {
    std::string port;
    std::optional<qnbot::Side> side;
    std::uint32_t sample_rate{10};
    qnbot::DebugDetail detail{qnbot::DebugDetail::summary};
    std::string target_name{example::default_target_name};
    std::string package_id;
};

qnbot::DebugDetail parse_detail(const std::string& text) {
    if (text == "summary") return qnbot::DebugDetail::summary;
    if (text == "full") return qnbot::DebugDetail::full;
    throw std::invalid_argument("--detail must be summary or full");
}

Options parse_options(int argc, char** argv) {
    Options options;
    for (int index = 1; index < argc; ++index) {
        const std::string argument = argv[index];
        if (argument == "--port") {
            options.port = example::require_value(argc, argv, index, argument);
        } else if (argument == "--side") {
            options.side = example::parse_side(
                example::require_value(argc, argv, index, argument));
        } else if (argument == "--sample-rate") {
            options.sample_rate =
                static_cast<std::uint32_t>(example::parse_positive_count(
                    example::require_value(argc, argv, index, argument),
                    argument));
        } else if (argument == "--detail") {
            options.detail = parse_detail(
                example::require_value(argc, argv, index, argument));
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
    target.name = options.target_name;
    target.side = options.side;
    target.source =
        qnbot::DeviceSelector{"glove", std::string("primary"), std::nullopt};
    target.algorithms = {qnbot::TargetAlgorithm{options.package_id}};
    config.targets.push_back(std::move(target));
    config.debug = qnbot::DebugConfig{
        {qnbot::DebugModule::transport, qnbot::DebugModule::device,
         qnbot::DebugModule::capture, qnbot::DebugModule::calibration,
         qnbot::DebugModule::retargeting, qnbot::DebugModule::output},
        options.detail,
        options.sample_rate};
    return config;
}

} // namespace

int main(int argc, char** argv) {
    try {
        const auto options = parse_options(argc, argv);
        qnbot::Sdk sdk(make_config(options));
        auto glove = sdk.glove();

        auto device = glove.device("primary");
        auto pose = device.pose();
        auto output = device.output(options.target_name);
        const auto pose_subscription =
            pose.subscribe([](const qnbot::Sample<qnbot::GlovePose>& sample) {
                std::cout << "pose sequence=" << sample.sequence << '\n';
            });
        const auto output_subscription =
            output.subscribe(example::print_output);

        if (std::signal(SIGINT, record_interruption) == SIG_ERR) {
            throw std::runtime_error("cannot install Ctrl+C handler");
        }
        glove.start();

        std::atomic<bool> runner_done{false};
        std::exception_ptr runner_error;
        std::thread runner([&] {
            try {
                glove.run_forever();
            } catch (...) {
                runner_error = std::current_exception();
            }
            runner_done.store(true, std::memory_order_release);
        });
        std::cout << "ready; press Ctrl+C to stop and close the Debug session"
                  << std::endl;

        while (!interrupted.load(std::memory_order_relaxed) &&
               !runner_done.load(std::memory_order_acquire)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        if (interrupted.load(std::memory_order_relaxed)) {
            // The foreground runner may not have entered its stop-admission state yet.
            while (!runner_done.load(std::memory_order_acquire)) {
                try {
                    glove.request_stop();
                    break;
                } catch (const qnbot::LifecycleError&) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
            }
        }
        runner.join();
        std::exception_ptr close_error;
        try {
            sdk.close();
        } catch (...) {
            close_error = std::current_exception();
        }
        static_cast<void>(pose_subscription);
        static_cast<void>(output_subscription);
        if (runner_error) std::rethrow_exception(runner_error);
        if (close_error) std::rethrow_exception(close_error);
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        return example::report_error(error);
    } catch (...) {
        return example::report_error(std::runtime_error("unknown error"));
    }
}
