#include "example_support.hpp"

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>
#include <unordered_set>

namespace {

qnbot::SdkConfig make_config(const example::SerialOptions& options) {
    auto config = example::runtime_config(options);
    config.algorithms.capture.interaction =
        qnbot::CaptureInteractionMode::external;
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

void wait_for_output(
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

} // namespace

int main(int argc, char** argv) {
    try {
        const auto options = example::parse_serial_options(
            argc, argv, example::SerialExample::background);
        qnbot::Sdk sdk(make_config(options));
        auto glove = sdk.glove();

        auto device = glove.device("primary");
        auto output = device.output(options.target_name);
        auto capture_progress = device.capture_progress();
        auto calibration_progress =
            device.calibration_progress(options.target_name);
        auto capture_control = device.capture_control();
        const auto subscription = output.subscribe(example::print_output);
        glove.start();
        glove.run_background();
        wait_for_output(output, capture_progress, capture_control,
                        calibration_progress);
        std::this_thread::sleep_for(
            std::chrono::duration<double>(options.seconds));
        example::print_health("running", glove.health());

        glove.request_stop();
        glove.join();
        glove.close();
        static_cast<void>(subscription);
        std::cout << "background runtime complete\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        return example::report_error(error);
    }
}
