#include <qnbot/glove.hpp>

#include "example_support.hpp"

#include <cstdlib>
#include <iostream>
#include <optional>
#include <string>

int main(int argc, char** argv) {
    try {
        std::string port;
        std::optional<qnbot::Side> side;
        for (int index = 1; index < argc; ++index) {
            const std::string argument = argv[index];
            if (argument == "--port") {
                port = example::require_value(argc, argv, index, argument);
            } else if (argument == "--side") {
                side = example::parse_side(
                    example::require_value(argc, argv, index, argument));
            } else {
                throw std::invalid_argument("unknown argument: " + argument);
            }
        }
        if (port.empty()) throw std::invalid_argument("--port is required");
        if (!side) throw std::invalid_argument("--side is required");

        qnbot::Sdk sdk(example::serial_config(port, *side));
        auto glove = sdk.glove();

        auto skeleton = glove.device().skeleton();
        const auto joint_angles_subscription =
            skeleton.joint_angles().subscribe(
                [](const qnbot::Sample<qnbot::HandJointCommand>& sample) {
                    std::cout << "skeleton sequence=" << sample.sequence
                              << " target=" << sample.value.target
                              << " joints=" << sample.value.joints.size()
                              << '\n';
                });
        const auto pose_subscription = skeleton.pose().subscribe(
            [](const qnbot::Sample<qnbot::HandSkeletonPose>& sample) {
                const auto& pose = sample.value;
                std::cout << "pose sequence=" << sample.sequence
                          << " frame=" << pose.coordinate_frame << " wrist=["
                          << pose.positions_local[0][0] << ", "
                          << pose.positions_local[0][1] << ", "
                          << pose.positions_local[0][2] << "] index_tip=["
                          << pose.positions_local[9][0] << ", "
                          << pose.positions_local[9][1] << ", "
                          << pose.positions_local[9][2] << "]\n";
            });

        glove.start();
        std::cout << "running; press Ctrl+C to stop" << std::endl;
        glove.run_forever();
        static_cast<void>(joint_angles_subscription);
        static_cast<void>(pose_subscription);
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        return example::report_error(error);
    }
}
