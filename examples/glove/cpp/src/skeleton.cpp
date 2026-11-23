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
            const auto [option, value] =
                example::next_argument(argc, argv, index);
            if (option == "--port")
                port = value;
            else if (option == "--side")
                side = example::parse_side(value);
            else
                throw std::invalid_argument("unknown argument: " + option);
        }
        if (port.empty()) throw std::invalid_argument("--port is required");
        if (!side) throw std::invalid_argument("--side is required");

        qnbot::Sdk sdk(example::serial_config(port, *side));
        auto glove = sdk.glove();

        auto device = glove.device();
        auto skeleton = device.skeleton();
        const auto joint_angles_subscription =
            skeleton.joint_angles().subscribe(
                [](const qnbot::Sample<qnbot::HandJointCommand>& sample) {
                    std::cout << "skeleton sequence=" << sample.sequence
                              << " target=" << sample.value.target
                              << " joints=" << sample.value.joints.size()
                              << '\n';
                });
        glove.start();
        std::cout << "running; press Ctrl+C to stop" << std::endl;
        glove.run_forever();
        static_cast<void>(joint_angles_subscription);
        glove.close();
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        return example::report_error(error);
    }
}
