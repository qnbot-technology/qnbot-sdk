#include <qnbot/glove.hpp>

#include <chrono>
#include <csignal>
#include <cstdlib>
#include <exception>
#include <iomanip>
#include <iostream>
#include <optional>
#include <thread>
#include <utility>

namespace {

volatile std::sig_atomic_t stop_requested = 0;

void handle_sigint(int) { stop_requested = 1; }

void print_source_time(const std::optional<double>& source_time) {
    std::cout << "source_time_ms=";
    if (source_time)
        std::cout << *source_time;
    else
        std::cout << "none";
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
        std::cout << std::setprecision(16);
        qnbot::SdkConfig config;
        config.devices = {qnbot::GloveConfig{}};

        qnbot::Sdk sdk(std::move(config));
        auto glove = sdk.glove();
        const double reference_time_ms = external_reference_time_ms();
        glove.sync_time(reference_time_ms);

        auto device = glove.device();
        const auto subscription = device.pose().subscribe(
            [](const qnbot::Sample<qnbot::GlovePose>& sample) {
                if (!sample.value.payload.hand) return;
                const auto& fingertips = sample.value.payload.hand->fingertips;
                const auto& index = fingertips.at(qnbot::GloveFinger::index);
                std::cout << "pose sequence=" << sample.sequence << ' ';
                print_source_time(sample.source_time);
                std::cout << " observed_at_ms=" << sample.observed_at
                          << " fingertips=" << fingertips.size()
                          << " index_position=(" << index.position[0] << ", "
                          << index.position[1] << ", " << index.position[2]
                          << ") index_quaternion_xyzw=("
                          << index.quaternion_xyzw[0] << ", "
                          << index.quaternion_xyzw[1] << ", "
                          << index.quaternion_xyzw[2] << ", "
                          << index.quaternion_xyzw[3] << ")\n";
            });

        const auto status_subscription = device.status().subscribe(
            [](const qnbot::Sample<qnbot::GloveStatus>& sample) {
                std::cout << "status ";
                print_source_time(sample.source_time);
                std::cout << " observed_at_ms=" << sample.observed_at
                          << " connected=" << sample.value.connected << '\n';
            });

        glove.start();
        glove.run_background();
        std::cout << "Glove time synchronized to " << reference_time_ms
                  << " ms; resynchronizing every 1 s; press Ctrl+C to stop"
                  << std::endl;
        while (!stop_requested) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
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
        static_cast<void>(status_subscription);
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "qnbot time sync example failed: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
