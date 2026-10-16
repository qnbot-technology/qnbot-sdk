#include "example_support.hpp"

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>

int main(int argc, char** argv) {
    try {
        const auto options = example::parse_serial_options(
            argc, argv, example::SerialExample::manual);
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
        auto output = device.output(example::default_target_name);
        glove.start();

        for (std::uint64_t index = 0; index < options.updates; ++index) {
            const auto update = glove.update();
            std::cout << "tick=" << update.tick()
                      << " tasks=" << update.ran_task_count() << '\n';
            if (update.sleep() == qnbot::SleepResult::no_tasks)
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            if (const auto latest = output.latest()) {
                example::print_output(*latest);
            }
        }

        glove.close();
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        return example::report_error(error);
    }
}
