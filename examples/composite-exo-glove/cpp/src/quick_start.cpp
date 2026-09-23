#include "composite_example_support.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>

int main(int argc, char** argv) {
    try {
        const auto port = example::parse_port(argc, argv);
        qnbot::Sdk sdk(example::composite_config(port));
        auto glove_domain = sdk.glove();
        auto exo_domain = sdk.exo();
        auto glove = glove_domain.device("glove");
        auto exo = exo_domain.device("exo");
        const auto pose_subscription = glove.pose().subscribe(
            [](const qnbot::Sample<qnbot::GlovePose>& sample) {
                std::cout << "glove pose sequence=" << sample.sequence
                          << " fingertips="
                          << sample.value.payload.fingertip_local.size()
                          << '\n';
            });
        const auto telemetry_subscription = exo.telemetry().subscribe(
            [](const qnbot::Sample<qnbot::ExoTelemetry>& sample) {
                std::cout << "exo telemetry sequence=" << sample.sequence
                          << " left_arm_joints=";
                example::print_values(
                    sample.value.payload.left_arm.joint_positions_rad);
                std::cout << '\n';
            });

        glove_domain.start();
        exo_domain.start();
        std::cout << "running shared composite link; press Ctrl+C to stop\n";
        // The aggregate runner is required here because this example drives
        // both member domains through one SDK instance.
        sdk.run_forever();
        static_cast<void>(pose_subscription);
        static_cast<void>(telemetry_subscription);
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        return example::report_error(error);
    }
}
