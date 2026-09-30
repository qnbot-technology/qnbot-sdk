#include "example_support.hpp"

#include <cstdlib>
#include <iostream>

int main(int argc, char** argv) {
    try {
        const auto options = example::parse_serial_options(
            argc, argv, example::SerialExample::foreground);
        const auto side = *options.side;
        qnbot::SdkConfig config;
        qnbot::SerialConnection connection;
        connection.port = options.port;
        config.devices.push_back(qnbot::GloveConfig{side, connection});
        qnbot::TargetConfig target;
        target.name = example::default_target_name;
        target.side = side;
        target.source = qnbot::DeviceSelector{"glove", std::nullopt, side};
        target.algorithms = {qnbot::TargetAlgorithm{options.package_id}};
        config.targets.push_back(target);
        qnbot::Sdk sdk(config);
        auto glove = sdk.glove();

        auto device = glove.device();
        const auto subscription = device.output(example::default_target_name)
                                      .subscribe(example::print_output);
        glove.start();
        try {
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
