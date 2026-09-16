#include <qnbot/coro.hpp>

#include "example_coroutine_task.hpp"
#include "example_support.hpp"

#include <cstdlib>
#include <chrono>
#include <iostream>
#include <memory>
#include <optional>
#include <stop_token>
#include <string>
#include <thread>
#include <unordered_set>
#include <utility>

namespace {

struct Options {
    std::string port;
    std::optional<qnbot::Side> side;
    std::uint64_t samples{10};
    std::string target_name{example::default_target_name};
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
        } else if (argument == "--samples") {
            options.samples = example::parse_positive_count(
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
    if (options.port.empty()) throw std::invalid_argument("--port is required");
    if (!options.side) {
        throw std::invalid_argument("--side is required with --port");
    }
    if (options.package_id.empty()) {
        throw std::invalid_argument("--package-id is required");
    }
    return options;
}

qnbot::SdkConfig make_config(const Options& options) {
    example::SerialOptions runtime;
    runtime.port = options.port;
    runtime.side = options.side;
    runtime.target_name = options.target_name;
    runtime.package_id = options.package_id;
    auto config = example::runtime_config(runtime);
    config.algorithms.calibration.interaction =
        qnbot::CalibrationInteractionMode::external;
    return config;
}

void handle_capture_prompt(
    const qnbot::ReadChannel<qnbot::CaptureProgress>& capture_progress,
    const qnbot::CaptureControl& capture_control,
    const qnbot::ReadChannel<qnbot::CalibrationProgress>& calibration_progress,
    std::unordered_set<std::string>& handled_request_ids) {
    if (const auto capture = capture_progress.latest()) {
        if (capture->value.session_state ==
            qnbot::CaptureSessionState::failed) {
            const auto message = capture->value.failure
                                     ? capture->value.failure->message
                                     : "unknown error";
            throw std::runtime_error("capture failed: " + message);
        }
        const auto& stage = capture->value.stage;
        if (stage.state == qnbot::CaptureStageState::awaiting_confirmation &&
            stage.request_id &&
            handled_request_ids.count(*stage.request_id) == 0) {
            std::cout << stage.prompt << " [Y/n]: ";
            std::string answer;
            std::getline(std::cin, answer);
            if (answer.empty() || answer == "y" || answer == "Y" ||
                answer == "yes" || answer == "YES") {
                capture_control.confirm(*stage.request_id);
            } else {
                capture_control.cancel(*stage.request_id);
                throw std::runtime_error("capture was cancelled");
            }
            handled_request_ids.insert(*stage.request_id);
        }
    }
    if (const auto calibration = calibration_progress.latest();
        calibration &&
        calibration->value.job.state == qnbot::CalibrationJobState::failed) {
        const auto message = calibration->value.job.failure
                                 ? calibration->value.job.failure->message
                                 : "unknown error";
        throw std::runtime_error("calibration failed: " + message);
    }
}

void wait_until_ready(
    const qnbot::ReadChannel<qnbot::HandJointCommand>& output,
    const qnbot::ReadChannel<qnbot::CaptureProgress>& capture_progress,
    const qnbot::CaptureControl& capture_control,
    const qnbot::ReadChannel<qnbot::CalibrationProgress>&
        calibration_progress) {
    std::unordered_set<std::string> handled_request_ids;
    const auto deadline =
        std::chrono::steady_clock::now() + std::chrono::seconds(300);
    while (!output.latest() && std::chrono::steady_clock::now() < deadline) {
        handle_capture_prompt(capture_progress, capture_control,
                              calibration_progress, handled_request_ids);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    if (!output.latest()) {
        throw std::runtime_error(
            "retargeting output was not ready before the timeout");
    }
}

example::Task<qnbot::Sample<qnbot::HandJointCommand>>
wait_for_output(qnbot::ReadChannel<qnbot::HandJointCommand> channel,
                std::shared_ptr<qnbot::coro::DeferredExecutor> executor,
                std::stop_token stop_token) {
    co_return co_await qnbot::coro::next(channel.next(), std::move(executor),
                                         stop_token);
}

} // namespace

int main(int argc, char** argv) {
    try {
        const auto options = parse_options(argc, argv);
        qnbot::Sdk sdk(make_config(options));
        auto glove = sdk.glove();
        auto device = glove.device("primary");
        auto output = device.output(options.target_name);
        auto capture_progress = device.capture_progress();
        auto calibration_progress =
            device.calibration_progress(options.target_name);
        auto capture_control = device.capture_control();
        auto executor = std::make_shared<example::QueueExecutor>();

        glove.start();
        glove.run_background();
        wait_until_ready(output, capture_progress, capture_control,
                         calibration_progress);
        for (std::uint64_t index = 0; index < options.samples; ++index) {
            auto pending = wait_for_output(output, executor, std::stop_token{});
            example::print_output(pending.get());
        }

        glove.request_stop();
        glove.join();
        glove.close();
        std::cout << "async runtime complete\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        return example::report_error(error);
    }
}
