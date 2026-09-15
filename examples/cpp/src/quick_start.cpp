#include <qnbot/glove.hpp>

#include <cstdlib>
#include <exception>
#include <iostream>
#include <utility>

int main() {
    try {
        qnbot::SdkConfig config;
        config.devices = {qnbot::GloveConfig{}};

        qnbot::Sdk sdk(std::move(config));
        auto glove = sdk.glove();

        auto pose = glove.device().pose();
        const auto subscription =
            pose.subscribe([](const qnbot::Sample<qnbot::GlovePose>& sample) {
                std::cout << "pose sequence=" << sample.sequence
                          << " fingertips="
                          << sample.value.payload.fingertip_local.size()
                          << '\n';
            });

        glove.start();
        std::cout << "running; press Ctrl+C to stop" << std::endl;
        glove.run_forever();
        static_cast<void>(subscription);
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "qnbot quick start failed: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
