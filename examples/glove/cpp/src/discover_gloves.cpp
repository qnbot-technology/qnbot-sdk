#include <qnbot/glove.hpp>

#include <cstdlib>
#include <iostream>

int main() {
    try {
        const auto devices = qnbot::discover_gloves();
        std::cout << "discovered records=" << devices.size() << '\n';
        for (const auto& device : devices) {
            const auto hand = !device.hand                        ? "unknown"
                              : *device.hand == qnbot::Side::left ? "left"
                                                                  : "right";
            std::cout << "transport_type="
                      << qnbot::glove_transport_type_name(device.transport_type)
                      << " port=" << device.port << " hand=" << hand
                      << " sn=" << (device.sn ? *device.sn : "unknown")
                      << " status=" << (device.error ? "error" : "ok");
            if (device.error) {
                std::cout << " message=" << device.error->message;
            }
            std::cout << '\n';
        }

        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
