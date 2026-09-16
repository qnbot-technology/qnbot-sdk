#include <qnbot/glove.hpp>

#include "external_input_support.hpp"

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>
#include <unordered_set>

namespace {

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

} // namespace

int main(int argc, char** argv) {
    try {
        const auto package_id =
            external_input_example::parse_package_id(argc, argv);
        qnbot::Sdk sdk(external_input_example::make_config(package_id));
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
            const auto pushed =
                device.push_frame(external_input_example::frame(step));
            if (step == 1) {
                std::cout << "pushed pose sequence="
                          << pushed.meta.sequence.value_or(0) << '\n';
            }
            const auto update = glove.update();
            handle_capture_prompt(capture_progress, capture_control,
                                  calibration_progress, confirmed_request_ids);
            ++step;
            if (update.has_next_task())
                static_cast<void>(update.sleep());
            else
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        const auto latest_pose = pose.latest();
        const auto latest_output = output.latest();
        if (!latest_pose || !latest_output) {
            throw std::runtime_error(
                "external input did not publish retargeting output");
        }
        std::cout << "latest pose sequence=" << latest_pose->sequence << '\n';
        example::print_output(*latest_output);
        glove.close();
        std::cout << "external manual input complete\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        return example::report_error(error);
    }
}
