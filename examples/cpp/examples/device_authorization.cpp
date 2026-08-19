#include "device_authorization_cleanup.hpp"
#include "example_support.hpp"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {

struct AuthorizationOptions {
    std::string port;
    std::optional<qnbot::Side> side;
    std::string operation;
    std::string ticket_path;
    std::string signature_path;
    bool validate_only = false;
};

AuthorizationOptions parse_options(int argc, char** argv) {
    AuthorizationOptions options;
    for (int index = 1; index < argc; ++index) {
        const std::string argument = argv[index];
        if (argument == "--port") {
            options.port = example::require_value(argc, argv, index, argument);
        } else if (argument == "--side") {
            options.side = example::parse_side(
                example::require_value(argc, argv, index, argument));
        } else if (argument == "--operation") {
            options.operation =
                example::require_value(argc, argv, index, argument);
        } else if (argument == "--ticket") {
            options.ticket_path =
                example::require_value(argc, argv, index, argument);
        } else if (argument == "--signature") {
            options.signature_path =
                example::require_value(argc, argv, index, argument);
        } else if (argument == "--validate") {
            options.validate_only = true;
        } else {
            throw std::invalid_argument("unknown argument: " + argument);
        }
    }
    if (options.port.empty()) throw std::invalid_argument("--port is required");
    if (!options.side) {
        throw std::invalid_argument("--side is required with --port");
    }
    if (options.operation != "permanent" && options.operation != "deactivate" &&
        options.operation != "factory") {
        throw std::invalid_argument(
            "--operation must be permanent, deactivate, or factory");
    }
    if (options.ticket_path.empty())
        throw std::invalid_argument("--ticket is required");
    if (options.signature_path.empty()) {
        throw std::invalid_argument("--signature is required");
    }
    return options;
}

std::vector<std::uint8_t> read_binary_file(const std::string& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input)
        throw std::runtime_error("unable to open authorization response file");
    return {std::istreambuf_iterator<char>(input),
            std::istreambuf_iterator<char>()};
}

std::pair<std::vector<std::uint8_t>, std::vector<std::uint8_t>>
load_service_response(const qnbot::GloveActivationChallenge& challenge,
                      const AuthorizationOptions& options) {
    // Replace these file reads with the application's authenticated service
    // client. Pass the Challenge and operation; keep Ticket/signature opaque.
    static_cast<void>(challenge);
    return {read_binary_file(options.ticket_path),
            read_binary_file(options.signature_path)};
}

} // namespace

int main(int argc, char** argv) {
    try {
        const auto options = parse_options(argc, argv);
        qnbot::Sdk sdk(example::serial_config(options.port, *options.side));
        example::Cleanup cleanup;
        std::optional<qnbot::Glove> glove;
        std::optional<qnbot::GloveDevice> selected_device;
        bool connected = false;
        bool started = false;
        bool factory_active = false;
        try {
            glove.emplace(sdk.glove());
            if (!options.validate_only) {
                glove->connect();
                connected = true;
                selected_device.emplace(glove->device("primary"));
                auto& device = *selected_device;
                const auto info = device.get_device_info();
                const auto status = device.activation_status();
                std::cout << "device model=" << info.model
                          << " firmware=" << info.firmware_version
                          << " activated=" << status.activated << '\n';

                const auto challenge = device.activation_challenge();
                auto service_response =
                    load_service_response(challenge, options);
                if (options.operation == "permanent") {
                    const auto result = device.activate_permanently(
                        qnbot::ByteView(service_response.first),
                        qnbot::ByteView(service_response.second));
                    std::cout
                        << "permanent activated=" << result.status.activated
                        << '\n';
                } else if (options.operation == "deactivate") {
                    const auto result = device.revoke_permanent_activation(
                        qnbot::ByteView(service_response.first),
                        qnbot::ByteView(service_response.second));
                    std::cout
                        << "permanent deactivated=" << !result.status.activated
                        << '\n';
                } else {
                    const auto factory = device.authorize_factory_calibration(
                        qnbot::ByteView(service_response.first),
                        qnbot::ByteView(service_response.second));
                    factory_active = factory.active;
                    std::cout << "factory active=" << factory.active << '\n';
                }

                if (options.operation != "deactivate") {
                    device.start();
                    started = true;
                    std::cout << "telemetry started\n";
                    device.stop();
                    started = false;
                    std::cout << "telemetry stopped\n";
                }
                if (factory_active) {
                    static_cast<void>(device.exit_factory_calibration());
                    factory_active = false;
                }
                device.disconnect();
                connected = false;
            }
        } catch (...) {
            cleanup.capture_current("device authorization");
        }

        static_cast<void>(example::cleanup_authorization_session(
            cleanup, selected_device ? &*selected_device : nullptr,
            {started, factory_active, connected},
            [&] {
                if (glove) glove->close();
            },
            [&] { sdk.close(); }));
        cleanup.rethrow_if_failed();
        if (options.validate_only) {
            std::cout << "device authorization arguments valid\n";
        }
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        return example::report_error(error);
    }
}
