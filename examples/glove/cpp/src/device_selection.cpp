#include <qnbot/glove.hpp>

#include <cstdlib>
#include <exception>
#include <iostream>
#include <utility>

int main() {
    try {
        qnbot::SdkConfig config;
        config.devices = {
            qnbot::GloveConfig{"primary", qnbot::Side::left,
                               qnbot::ExternalConnection{}, true},
            qnbot::GloveConfig{"primary", qnbot::Side::right,
                               qnbot::ExternalConnection{}, true},
            qnbot::GloveConfig{"backup", qnbot::Side::left,
                               qnbot::ExternalConnection{}, true},
        };
        qnbot::Sdk sdk(std::move(config));
        auto glove = sdk.glove();

        const auto primary_left = glove.device("primary", qnbot::Side::left);
        const auto primary_right = glove.device("primary", qnbot::Side::right);
        const auto backup_left = glove.device("backup");
        std::cout << "primary left source_id=" << primary_left.source_id()
                  << '\n';
        std::cout << "primary right source_id=" << primary_right.source_id()
                  << '\n';
        std::cout << "backup left source_id=" << backup_left.source_id()
                  << '\n';

        try {
            glove.device("primary");
        } catch (const qnbot::AmbiguousSelectionError&) {
            std::cout << "expected ambiguity: name=primary\n";
        }
        try {
            glove.device(qnbot::Side::left);
        } catch (const qnbot::AmbiguousSelectionError&) {
            std::cout << "expected ambiguity: side=left\n";
        }

        glove.close();
        std::cout << "device selection complete\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "qnbot device selection failed: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
