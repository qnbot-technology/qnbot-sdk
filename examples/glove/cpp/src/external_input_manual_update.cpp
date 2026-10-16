#include <qnbot/glove.hpp>

#include "external_input_support.hpp"

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>
#include <unordered_set>

namespace {

void check_preparation(
    const qnbot::ReadChannel<qnbot::CaptureProgress>& capture_progress,
    const qnbot::ReadChannel<qnbot::CalibrationProgress>&
        calibration_progress) {
    if (const auto capture = capture_progress.latest()) {
        if (capture->value.session_state ==
            qnbot::CaptureSessionState::failed) {
            const auto message = capture->value.failure
                                     ? capture->value.failure->message
                                     : "unknown error";
            throw std::runtime_error("capture failed: " + message);
        }
        if (capture->value.session_state ==
            qnbot::CaptureSessionState::cancelled)
            throw std::runtime_error("capture was cancelled");
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

void handle_capture_prompt(
    const qnbot::ReadChannel<qnbot::CaptureProgress>& capture_progress,
    const qnbot::CaptureControl& capture_control,
    std::unordered_set<std::string>& handled_request_ids) {
    if (const auto capture = capture_progress.latest()) {
        const auto& stage = capture->value.stage;
        if (stage.state == qnbot::CaptureStageState::awaiting_confirmation &&
            stage.request_id &&
            handled_request_ids.count(*stage.request_id) == 0) {
            std::cout << (stage.prompt_en ? *stage.prompt_en : stage.prompt)
                      << " [Y/n]: ";
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
}

} // namespace

int main(int argc, char** argv) {
    try {
        const auto options = external_input_example::parse_options(argc, argv);
        qnbot::Sdk sdk(external_input_example::make_config(options));
        auto glove = sdk.glove();

        auto device = glove.device("external");
        auto pose = device.pose();
        auto output = device.output("hand");
        auto capture_progress = device.capture_progress();
        auto calibration_progress = device.calibration_progress("hand");
        auto capture_control = device.capture_control();
        std::unordered_set<std::string> confirmed_request_ids;
        device.start();

        const auto deadline =
            std::chrono::steady_clock::now() + std::chrono::seconds(60);
        std::uint64_t step = 1;
        while (!output.latest() &&
               std::chrono::steady_clock::now() < deadline) {
            check_preparation(capture_progress, calibration_progress);
            device.push_frame(external_input_example::frame(step));
            const auto update = glove.update();
            if (options.interaction ==
                qnbot::CaptureInteractionMode::external) {
                handle_capture_prompt(capture_progress, capture_control,
                                      confirmed_request_ids);
            }
            ++step;
            if (update.sleep() == qnbot::SleepResult::no_tasks)
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        const auto latest_pose = pose.latest();
        const auto latest_output = output.latest();
        if (!latest_pose || !latest_output) {
            throw std::runtime_error(
                "external input did not publish retargeting output");
        }
        example::print_output(*latest_output);
        glove.close();
        std::cout << "external manual input complete\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        return example::report_error(error);
    }
}
