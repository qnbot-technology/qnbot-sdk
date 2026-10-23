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
            auto skeleton = device.skeleton();
            subscriptions.push_back(skeleton.joint_angles().subscribe(
                [label](const qnbot::Sample<qnbot::HandJointCommand>& sample) {
                    std::cout
                        << label << " skeleton sequence=" << sample.sequence
                        << " target=" << sample.value.target
                        << " joints=" << sample.value.joints.size() << '\n';
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
