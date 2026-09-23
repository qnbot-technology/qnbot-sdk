#include "composite_example_support.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>

int main(int argc, char** argv) {
    try {
        qnbot::Sdk sdk(
            example::composite_config(example::parse_port(argc, argv)));
        auto glove_domain = sdk.glove();
        auto exo_domain = sdk.exo();
        auto glove = glove_domain.device("glove");
        auto exo = exo_domain.device("exo");
        const auto pose_subscription = glove.pose().subscribe(
            [](const qnbot::Sample<qnbot::GlovePose>& sample) {
                std::cout << "glove pose sequence=" << sample.sequence
                          << " fingers="
                          << sample.value.payload.fingertip_local.size()
                          << '\n';
            });
        const auto glove_status_subscription = glove.status().subscribe(
            [](const qnbot::Sample<qnbot::GloveStatus>& sample) {
                std::cout << "glove status connected=" << sample.value.connected
                          << " stale=" << sample.value.stale << '\n';
            });
        const auto exo_subscription = exo.telemetry().subscribe(
            [](const qnbot::Sample<qnbot::ExoTelemetry>& sample) {
                std::cout << "exo telemetry sequence=" << sample.sequence
                          << " left_arm=";
                example::print_values(
                    sample.value.payload.left_arm.joint_positions_rad);
                std::cout << '\n';
            });
        const auto exo_status_subscription = exo.status().subscribe(
            [](const qnbot::Sample<qnbot::DeviceStatus>& sample) {
                std::cout << "exo status connected=" << sample.value.connected
                          << " stale=" << sample.value.stale << '\n';
            });

        glove_domain.start();
        exo_domain.start();
        std::cout << "reading Glove pose, Exo telemetry, and both statuses; "
                     "press Ctrl+C to stop\n";
        // Drive both member domains through the aggregate runner.
        sdk.run_forever();
        static_cast<void>(pose_subscription);
        static_cast<void>(glove_status_subscription);
        static_cast<void>(exo_subscription);
        static_cast<void>(exo_status_subscription);
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        return example::report_error(error);
    }
}
