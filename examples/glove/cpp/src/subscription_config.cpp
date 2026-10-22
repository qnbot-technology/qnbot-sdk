#include "example_support.hpp"

#include <chrono>
#include <cstdlib>
#include <thread>

int main(int argc, char** argv) {
    try {
        std::string port;
        std::optional<qnbot::Side> side;
        std::string mode = "latest";
        std::size_t capacity = 128;
        double max_rate_hz = 30.0;
        double seconds = 10.0;
        for (int index = 1; index < argc; ++index) {
            const std::string argument = argv[index];
            const auto value =
                example::require_value(argc, argv, index, argument);
            if (argument == "--port")
                port = value;
            else if (argument == "--side")
                side = example::parse_side(value);
            else if (argument == "--mode")
                mode = value;
            else if (argument == "--capacity")
                capacity = example::parse_positive_count(value, argument);
            else if (argument == "--max-rate-hz")
                max_rate_hz = example::parse_positive_double(value, argument);
            else if (argument == "--seconds")
                seconds = example::parse_positive_double(value, argument);
            else
                throw std::invalid_argument("unknown argument: " + argument);
        }
        if (port.empty()) throw std::invalid_argument("--port is required");
        if (!side) throw std::invalid_argument("--side is required");
        if (mode != "latest" && mode != "fifo")
            throw std::invalid_argument("--mode must be latest or fifo");
        const auto config =
            mode == "latest"
                ? qnbot::SubscriptionConfig::latest(max_rate_hz)
                : qnbot::SubscriptionConfig::fifo(capacity, max_rate_hz);
        qnbot::Sdk sdk(example::serial_config(port, *side));
        auto glove = sdk.glove();
        auto device = glove.device();
        std::size_t count = 0;
        std::uint64_t last_sequence = 0;
        std::chrono::steady_clock::time_point first_at, last_at;
        const auto subscription = device.pose().with_config(config).subscribe(
            [&](const qnbot::Sample<qnbot::GlovePose>& sample) {
                last_at = std::chrono::steady_clock::now();
                if (count == 0) first_at = last_at;
                ++count;
                last_sequence = sample.sequence;
            });
        try {
            glove.start();
            glove.run_background();
            std::cout << "mode=" << mode << " max_rate_hz=" << max_rate_hz
                      << std::endl;
            std::this_thread::sleep_for(std::chrono::duration<double>(seconds));
            glove.request_stop();
            glove.join();
        } catch (...) {
            glove.close();
            throw;
        }
        static_cast<void>(subscription);
        glove.close();
        const double observed_hz =
            count > 1
                ? static_cast<double>(count - 1) /
                      std::chrono::duration<double>(last_at - first_at).count()
                : 0.0;
        std::cout << "samples=" << count << " observed_hz=" << observed_hz
                  << " last_sequence=" << last_sequence << '\n';
        std::cout << "subscription complete\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        return example::report_error(error);
    }
}
