#include <qnbot/glove.hpp>

#include <cstdlib>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>

int main(int argc, char** argv) {
    try {
        qnbot::GloveConfig glove_config;
        for (int i = 1; i < argc; ++i) {
            const std::string option = argv[i];
            if ((option != "--port" && option != "--side") || i + 1 >= argc)
                throw std::invalid_argument(
                    "usage: device_info [--port PORT] [--side left|right]");
            const std::string value = argv[++i];
            if (option == "--port") {
                qnbot::SerialConnection connection;
                connection.port = value;
                glove_config.connection = std::move(connection);
            } else if (value == "left")
                glove_config.side = qnbot::Side::left;
            else if (value == "right")
                glove_config.side = qnbot::Side::right;
            else
                throw std::invalid_argument("--side must be left or right");
        }
        qnbot::SdkConfig config;
        config.devices = {glove_config};
        qnbot::Sdk sdk(std::move(config));
        auto glove = sdk.glove();
        try {
            auto device = glove.device();
            const auto info = device.get_device_info();
            const auto display = [](const std::string& value) {
                return value.empty() ? "unknown" : value;
            };
            std::cout << "canonical_sn=" << info.canonical_sn
                      << " device_type=" << display(info.device_type)
                      << " model=" << display(info.model)
                      << " firmware_version=" << display(info.firmware_version)
                      << " hand="
                      << (info.hand == qnbot::Side::left ? "left" : "right")
                      << " hand_source=" << display(info.hand_source)
                      << " transport_type="
                      << qnbot::glove_transport_type_name(info.transport_type)
                      << std::endl;
        } catch (...) {
            glove.close();
            throw;
        }
        glove.close();
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "device info failed: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
