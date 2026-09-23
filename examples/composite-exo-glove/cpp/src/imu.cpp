#include "composite_example_support.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>

namespace {

void print_exo_imu(const qnbot::ExoTelemetry& telemetry) {
    if (telemetry.payload.torso_imu) {
        std::cout << "exo torso acceleration=";
        example::print_values(telemetry.payload.torso_imu->acceleration_mps2);
        std::cout << '\n';
    }
    if (telemetry.payload.extra_imu) {
        std::cout << "exo extra acceleration=";
        example::print_values(telemetry.payload.extra_imu->acceleration_mps2);
        std::cout << '\n';
    }
}

} // namespace

int main(int argc, char** argv) {
    try {
        qnbot::Sdk sdk(
            example::composite_config(example::parse_port(argc, argv)));
        auto glove_domain = sdk.glove();
        auto exo_domain = sdk.exo();
        auto glove = glove_domain.device("glove");
        auto exo = exo_domain.device("exo");
        const auto glove_subscription = glove.imu().subscribe(
            [](const qnbot::Sample<qnbot::GloveImu>& sample) {
                const auto& value = sample.value.payload;
                std::cout << "glove imu valid=" << value.valid << " gyro=["
                          << value.gyroscope_raw[0] << ", "
                          << value.gyroscope_raw[1] << ", "
                          << value.gyroscope_raw[2] << "]\n";
            });
        const auto exo_subscription = exo.telemetry().subscribe(
            [](const qnbot::Sample<qnbot::ExoTelemetry>& sample) {
                print_exo_imu(sample.value);
            });

        glove_domain.start();
        exo_domain.start();
        std::cout << "reading Glove and Exo IMU data; press Ctrl+C to stop\n";
        // Drive both member domains through the aggregate runner.
        sdk.run_forever();
        static_cast<void>(glove_subscription);
        static_cast<void>(exo_subscription);
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        return example::report_error(error);
    }
}
