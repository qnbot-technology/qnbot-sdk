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
        auto subscription = device.emf().subscribe(
            [](const qnbot::Sample<qnbot::GloveEmf>& sample) {
                const auto& payload = sample.value.payload;
                for (const auto& channel : payload.channels) {
                    std::cout << "emf session=" << payload.session
                              << " channel=" << unsigned(channel.channel_id)
                              << " units=" << payload.units;
                    if (payload.frame_sequence)
                        std::cout << " frame_sequence="
                                  << *payload.frame_sequence;
                    if (payload.sync) std::cout << " sync=" << *payload.sync;
                    auto print_matrix = [](const char* label,
                                           const auto& matrix) {
                        std::cout << " " << label << "=[";
                        for (const auto& row : matrix) {
                            std::cout << "[" << row[0] << "," << row[1] << ","
                                      << row[2] << "]";
                        }
                        std::cout << "]";
                    };
                    print_matrix("li", channel.li);
                    print_matrix("lq", channel.lq);
                    print_matrix("hi", channel.hi);
                    print_matrix("hq", channel.hq);
                    std::cout << "\n";
                }
            });

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
