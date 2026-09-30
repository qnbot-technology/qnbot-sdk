#include "composite_example_support.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>

int main(int argc, char** argv) {
    try {
        qnbot::Sdk sdk(
            example::composite_config(example::parse_port(argc, argv)));
        auto glove = sdk.glove();
        auto exo = sdk.exo();
        auto left_glove_device = glove.device(qnbot::Side::left);
        auto right_glove_device = glove.device(qnbot::Side::right);
        auto exo_device = exo.device();
        const auto left_pose_subscription = left_glove_device.pose().subscribe(
            [](const qnbot::Sample<qnbot::GlovePose>& sample) {
                std::cout << "left glove pose sequence=" << sample.sequence
                          << " fingers="
                          << sample.value.payload.fingertip_local.size()
                          << '\n';
            });
        const auto right_pose_subscription =
            right_glove_device.pose().subscribe(
                [](const qnbot::Sample<qnbot::GlovePose>& sample) {
                    std::cout << "right glove pose sequence=" << sample.sequence
                              << " fingers="
                              << sample.value.payload.fingertip_local.size()
                              << '\n';
                });
        const auto left_glove_status_subscription =
            left_glove_device.status().subscribe(
                [](const qnbot::Sample<qnbot::GloveStatus>& sample) {
                    std::cout << "left glove status connected="
                              << sample.value.connected
                              << " stale=" << sample.value.stale << '\n';
                });
        const auto right_glove_status_subscription =
            right_glove_device.status().subscribe(
                [](const qnbot::Sample<qnbot::GloveStatus>& sample) {
                    std::cout << "right glove status connected="
                              << sample.value.connected
                              << " stale=" << sample.value.stale << '\n';
                });
        const auto exo_subscription = exo_device.telemetry().subscribe(
            [](const qnbot::Sample<qnbot::ExoTelemetry>& sample) {
                std::cout << "exo telemetry sequence=" << sample.sequence
                          << " left_arm=";
                example::print_values(
                    sample.value.payload.left_arm.joint_positions_rad);
                std::cout << '\n';
            });
        const auto exo_status_subscription = exo_device.status().subscribe(
            [](const qnbot::Sample<qnbot::ExoStatus>& sample) {
                std::cout << "exo status connected=" << sample.value.connected
                          << " stale=" << sample.value.stale << '\n';
            });

        sdk.start();
        std::cout << "reading Glove pose, Exo telemetry, and both statuses; "
                     "press Ctrl+C to stop\n";
        // The root SDK lifecycle drives both member domains.
        sdk.run_forever();
        sdk.stop();
        sdk.close();
        static_cast<void>(left_pose_subscription);
        static_cast<void>(right_pose_subscription);
        static_cast<void>(left_glove_status_subscription);
        static_cast<void>(right_glove_status_subscription);
        static_cast<void>(exo_subscription);
        static_cast<void>(exo_status_subscription);
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        return example::report_error(error);
    }
}
