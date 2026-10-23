#include <qnbot/glove.hpp>

#include <cstdlib>
#include <exception>
#include <iostream>
#include <utility>
#include <vector>

int main() {
    try {
        qnbot::GloveConfig left;
        left.side = qnbot::Side::left;
        qnbot::GloveConfig right;
        right.side = qnbot::Side::right;
        qnbot::GloveReceiverConfig receiver;
        receiver.devices = {left, right};

        qnbot::SdkConfig config;
        config.devices = {receiver};
        qnbot::Sdk sdk(std::move(config));
        auto glove = sdk.glove();

        std::vector<qnbot::Subscription> subscriptions;
        for (const auto side : {qnbot::Side::left, qnbot::Side::right}) {
            const auto label = side == qnbot::Side::left ? "left" : "right";
            auto device = glove.device(side);
            subscriptions.push_back(device.pose().subscribe(
                [label](const qnbot::Sample<qnbot::GlovePose>& sample) {
                    std::cout << label << " pose sequence=" << sample.sequence
                              << " fingertips="
                              << sample.value.payload.fingertip_local.size()
                              << '\n';
                }));
        }

        try {
            glove.start();
            std::cout << "running; press Ctrl+C to stop" << std::endl;
            glove.run_forever();
        } catch (...) {
            glove.close();
            throw;
        }
        glove.close();
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "wireless Receiver failed: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
