#include <qnbot/glove.hpp>

#include <algorithm>
#include <cstdlib>
#include <iostream>

int main() {
    try {
        const auto devices = qnbot::discover_gloves();
        for (const auto& device : devices) {
            if (device.error) {
                std::cout << "port=" << device.port
                          << " error=" << device.error->message << '\n';
                continue;
            }
            const auto hand = !device.hand
                                  ? "unknown"
                                  : *device.hand == qnbot::Side::left ? "left"
                                                                       : "right";
            std::cout << "port=" << device.port << " hand=" << hand
                      << " sn=" << (device.sn ? *device.sn : "None") << '\n';
        }

        const auto selected =
            std::find_if(devices.begin(), devices.end(),
                         [](const auto& item) { return !item.error; });
        if (selected == devices.end() || !selected->hand) return EXIT_SUCCESS;

        qnbot::SerialConnection connection;
        connection.port = selected->port;
        const qnbot::GloveConfig config{*selected->hand, connection};
        std::cout << "selected port=" << selected->port << " hand="
                  << (config.side == qnbot::Side::left ? "left" : "right")
                  << '\n';
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
