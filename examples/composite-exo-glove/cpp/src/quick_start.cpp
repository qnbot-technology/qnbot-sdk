#include "composite_example_support.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>

int main() {
    try {
        qnbot::Sdk sdk(example::composite_config());
        auto glove = sdk.glove();
        auto exo = sdk.exo();
        auto left_glove_device = glove.device(qnbot::Side::left);
        auto right_glove_device = glove.device(qnbot::Side::right);
        auto exo_device = exo.device();
        const auto left_pose_subscription = left_glove_device.pose().subscribe(
            [](const qnbot::Sample<qnbot::GlovePose>& sample) {
                std::cout << "left glove pose sequence=" << sample.sequence
                          << " fingertips="
                          << sample.value.payload.fingertip_local.size()
                          << '\n';
            });
        const auto right_pose_subscription =
            right_glove_device.pose().subscribe(
                [](const qnbot::Sample<qnbot::GlovePose>& sample) {
                    std::cout << "right glove pose sequence=" << sample.sequence
                              << " fingertips="
                              << sample.value.payload.fingertip_local.size()
                              << '\n';
                });
        const auto telemetry_subscription = exo_device.telemetry().subscribe(
            [](const qnbot::Sample<qnbot::ExoTelemetry>& sample) {
                std::cout << "exo telemetry sequence=" << sample.sequence
                          << " left_arm_joints=";
                example::print_values(
                    sample.value.payload.left_arm.joint_positions_rad);
                std::cout << '\n';
            });

        sdk.start();
        std::cout << "running shared composite link; press Ctrl+C to stop\n";
        // There are no work targets in this example, so the taskless
        // runtime stays alive until the caller requests a stop.
        // The root SDK lifecycle drives both member domains.
        sdk.run_forever();
        sdk.stop();
        sdk.close();
        static_cast<void>(left_pose_subscription);
        static_cast<void>(right_pose_subscription);
        static_cast<void>(telemetry_subscription);
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        return example::report_error(error);
    }
}
