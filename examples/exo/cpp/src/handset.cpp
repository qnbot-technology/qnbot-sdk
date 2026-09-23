#include "exo_example_support.hpp"

#include <cstdlib>
#include <exception>
#include <iomanip>
#include <iostream>
#include <string>

namespace {

void print_handsets(const qnbot::Sample<qnbot::ExoTelemetry>& sample) {
    const auto& payload = sample.value.payload;
    std::cout << "handsets sequence=" << sample.sequence << " left=("
              << payload.left_handset.axis_x_raw << ", "
              << payload.left_handset.axis_y_raw
              << ", trigger=" << payload.left_handset.trigger_raw
              << ", buttons=0x" << std::hex << payload.left_handset.button_mask
              << std::dec << ") right=(" << payload.right_handset.axis_x_raw
              << ", " << payload.right_handset.axis_y_raw
              << ", trigger=" << payload.right_handset.trigger_raw
              << ", buttons=0x" << std::hex << payload.right_handset.button_mask
              << std::dec << ")\n";
}

} // namespace

int main(int argc, char** argv) {
    try {
        std::string port;
        std::string name = "primary";
        for (int index = 1; index < argc; ++index) {
            const std::string argument = argv[index];
            if (argument == "--port") {
                port = example::require_value(argc, argv, index, argument);
            } else if (argument == "--name") {
                name = example::require_value(argc, argv, index, argument);
            } else {
                throw std::invalid_argument("unknown argument: " + argument);
            }
        }
        if (port.empty()) throw std::invalid_argument("--port is required");

        qnbot::Sdk sdk(example::serial_config(port, name));
        auto exo = sdk.exo();
        auto device = exo.device(name);
        if (!device.get_device_info().capabilities.handsets) {
            std::cout << "this device reports no handset" << std::endl;
            exo.close();
            return EXIT_SUCCESS;
        }

        auto telemetry = device.telemetry();
        const auto subscription = telemetry.subscribe(print_handsets);

        exo.start();
        std::cout << "running; press Ctrl+C to stop" << std::endl;
        exo.run_forever();
        static_cast<void>(subscription);
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        return example::report_error(error);
    }
}
