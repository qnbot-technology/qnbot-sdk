#include "example_support.hpp"

#include <cstdlib>
#include <iostream>
#include <sstream>

namespace {

void print_pressure(const qnbot::Sample<qnbot::GlovePressure>& sample) {
    const auto& readings = sample.value.readings;
    std::ostringstream out;
    out << "pressure sequence=" << sample.sequence
        << " points=" << readings.size()
        << " first_raw=" << static_cast<unsigned>(readings.front()) << '\n';
    std::cout << out.str();
}

} // namespace

int main(int argc, char** argv) {
    try {
        if (argc != 3 || std::string(argv[1]) != "--port")
            throw std::invalid_argument("usage: pressure --port <port>");
        qnbot::GloveConfig glove_config;
        qnbot::SerialConnection connection;
        connection.port = argv[2];
        glove_config.connection = connection;
        qnbot::SdkConfig config;
        config.devices.push_back(glove_config);
        qnbot::Sdk sdk(config);
        auto glove = sdk.glove();
        auto device = glove.device();
        const auto subscription =
            device.pressure()
                .with_config(qnbot::SubscriptionConfig::latest(1.0))
                .subscribe(print_pressure);

        glove.start();
        std::cout << "running; press Ctrl+C to stop" << std::endl;
        glove.run_forever();
        static_cast<void>(subscription);
        glove.close();
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        return example::report_error(error);
    }
}
