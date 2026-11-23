#include "example_support.hpp"

#include <cstdlib>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

namespace {
const char* action_name(qnbot::CaptureQualityAction action) {
    static constexpr const char* names[] = {"next", "retry_stage",
                                            "recapture_reference"};
    return names[static_cast<std::size_t>(action)];
}
} // namespace

int main(int argc, char** argv) {
    try {
        std::string port;
        std::string stage = "open";
        std::optional<qnbot::Side> side;
        for (int index = 1; index < argc; ++index) {
            const auto [option, value] =
                example::next_argument(argc, argv, index);
            if (option == "--port")
                port = value;
            else if (option == "--side")
                side = example::parse_side(value);
            else if (option == "--stage")
                stage = value;
            else
                throw std::invalid_argument("unknown argument: " + option);
        }
        if (port.empty()) throw std::invalid_argument("--port is required");
        if (!side) throw std::invalid_argument("--side is required");

        qnbot::Sdk sdk(example::serial_config(port, *side));
        auto glove = sdk.glove();
        auto device = glove.device();
        auto detector = device.create_capture_quality_session();
        auto pose = device.pose();
        glove.start(qnbot::CalibrationMode::none);
        glove.run_background();

        std::vector<std::string> required{"close"};
        if (stage != "close") required.push_back(stage);
        bool accepted = true;
        for (const auto& current : required) {
            std::cout << "摆好 " << current << "，按 Enter 收集 "
                      << detector.required_sample_count() << " 帧："
                      << std::flush;
            std::string answer;
            std::getline(std::cin, answer);
            std::vector<qnbot::GloveHandPose> window;
            while (window.size() < detector.required_sample_count()) {
                const auto sample = pose.next().get();
                if (!sample.value.payload.hand) continue;
                const auto& hand = *sample.value.payload.hand;
                if (!hand.fresh) continue;
                bool valid = true;
                for (const auto finger :
                     {qnbot::GloveFinger::thumb, qnbot::GloveFinger::index,
                      qnbot::GloveFinger::middle, qnbot::GloveFinger::ring,
                      qnbot::GloveFinger::pinky}) {
                    valid = valid && hand.raw_valid_mask.at(finger) &&
                            !hand.held_mask.at(finger);
                }
                if (!valid) {
                    window.clear();
                    continue;
                }
                window.push_back(hand);
            }
            const auto result = detector.assess_stage(current, window);
            std::cout << "stage=" << current << " accepted=" << result.accepted
                      << " action=" << action_name(result.action)
                      << " reason=" << result.reason
                      << " reason_en=" << result.reason_en << '\n';
            if (!result.accepted) {
                accepted = false;
                break;
            }
        }
        glove.request_stop();
        glove.join();
        glove.close();
        detector.close();
        return accepted ? EXIT_SUCCESS : EXIT_FAILURE;
    } catch (const std::exception& error) {
        return example::report_error(error);
    }
}
