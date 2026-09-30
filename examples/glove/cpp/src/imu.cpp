#include "example_support.hpp"

#include <cstdlib>
#include <iostream>

int main(int argc, char** argv) {
    try {
        const auto options = example::parse_serial_options(
            argc, argv, example::SerialExample::imu);
        qnbot::Sdk sdk(example::serial_config(options.port, *options.side));
        auto glove = sdk.glove();

        auto device = glove.device();
        const auto subscription = device.imu().subscribe(
            [](const qnbot::Sample<qnbot::GloveImu>& sample) {
                const auto& value = sample.value.payload;
                std::cout << "imu sequence=" << sample.sequence
                          << " valid=" << value.valid << " gyroscope_raw=["
                          << value.gyroscope_raw[0] << ", "
                          << value.gyroscope_raw[1] << ", "
                          << value.gyroscope_raw[2] << "] accelerometer_raw=["
                          << value.accelerometer_raw[0] << ", "
                          << value.accelerometer_raw[1] << ", "
                          << value.accelerometer_raw[2] << "]\n";
            });

        try {
            glove.start();
            std::cout << "running; press Ctrl+C to stop" << std::endl;
            glove.run_forever();
        } catch (...) {
            glove.close();
            throw;
        }
        static_cast<void>(subscription);
        glove.close();
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        return example::report_error(error);
    }
}
