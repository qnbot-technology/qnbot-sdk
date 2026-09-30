#include <qnbot/glove.hpp>

#include <chrono>
#include <csignal>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <thread>
#include <utility>

namespace {

volatile std::sig_atomic_t stop_requested = 0;

void handle_sigint(int) { stop_requested = 1; }

const char* finger_name(qnbot::GloveFinger finger) {
    switch (finger) {
    case qnbot::GloveFinger::thumb: return "thumb";
    case qnbot::GloveFinger::index: return "index";
    case qnbot::GloveFinger::middle: return "middle";
    case qnbot::GloveFinger::ring: return "ring";
    case qnbot::GloveFinger::pinky: return "pinky";
    }
    return "unknown";
}

double external_reference_time_ms() {
    // This standalone example uses the host clock as a runnable placeholder.
    // Replace it with an absolute Unix timestamp supplied by the application's
    // external time source. The API argument is milliseconds, not nanoseconds.
    const auto now = std::chrono::system_clock::now().time_since_epoch();
    return std::chrono::duration<double, std::milli>(now).count();
}

} // namespace

int main() {
    try {
        std::signal(SIGINT, handle_sigint);
        qnbot::SdkConfig config;
        config.devices = {qnbot::GloveConfig{}};

        qnbot::Sdk sdk(std::move(config));
        auto glove = sdk.glove();
        const double reference_time_ms = external_reference_time_ms();
        glove.sync_time(reference_time_ms);

        auto device = glove.device();
        const auto subscription = device.pose().subscribe(
            [](const qnbot::Sample<qnbot::GlovePose>& sample) {
                std::cout << "pose sequence=" << sample.sequence
                          << " fingertips={";
                bool first = true;
                for (const auto& [finger, node] :
                     sample.value.payload.fingertip_local) {
                    if (!first) std::cout << ", ";
                    const auto& position = node.position;
                    const auto& quaternion = node.quaternion_xyzw;
                    std::cout << finger_name(finger)
                              << "=GloveNodePose(position=(" << position[0]
                              << ", " << position[1] << ", " << position[2]
                              << "), quaternion_xyzw=(" << quaternion[0] << ", "
                              << quaternion[1] << ", " << quaternion[2] << ", "
                              << quaternion[3] << "))";
                    first = false;
                }
                std::cout << "}\n";
            });

        glove.start();
        glove.run_background();
        std::cout << "Glove time synchronized to " << reference_time_ms
                  << " ms; resynchronizing every 1 s; press Ctrl+C to stop"
                  << std::endl;
        while (!stop_requested) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            if (stop_requested) break;
            const double updated_reference_time_ms =
                external_reference_time_ms();
            glove.sync_time(updated_reference_time_ms);
            std::cout << "Glove time resynchronized to "
                      << updated_reference_time_ms << " ms" << std::endl;
        }

        glove.request_stop();
        glove.join();
        glove.close();
        static_cast<void>(subscription);
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "qnbot time sync example failed: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
