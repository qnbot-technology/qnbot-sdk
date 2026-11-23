#include "example_support.hpp"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <optional>
#include <sstream>
#include <thread>

namespace {

void print_pressure(const qnbot::Sample<qnbot::GlovePressure>& sample,
                    std::optional<std::size_t> point) {
    const auto& readings = sample.value.readings;
    const auto peak = std::max_element(readings.begin(), readings.end());
    const auto peak_index = static_cast<std::size_t>(peak - readings.begin());
    const auto selected = point.value_or(peak_index);
    std::ostringstream out;
    out << std::fixed << std::setprecision(1)
        << "pressure sequence=" << sample.sequence
        << " points=" << readings.size() << " raw_min="
        << static_cast<unsigned>(
               *std::min_element(readings.begin(), readings.end()))
        << " raw_mean="
        << std::accumulate(readings.begin(), readings.end(), 0.0) /
               readings.size()
        << " raw_max=" << static_cast<unsigned>(*peak)
        << " peak_index=" << peak_index << " capped_points="
        << std::count_if(readings.begin(), readings.end(),
                         [](auto raw) { return raw >= 107; })
        << " point_index=" << selected << " point_row=" << selected / 20
        << " point_column=" << selected % 20
        << " point_raw=" << static_cast<unsigned>(readings[selected])
        << " point_estimated_kpa=" << sample.value.estimated_kpa[selected]
        << '\n';
    std::cout << out.str() << std::flush;
}

} // namespace

int main(int argc, char** argv) {
    try {
        if ((argc != 3 && argc != 5) || std::string(argv[1]) != "--port" ||
            (argc == 5 && std::string(argv[3]) != "--point"))
            throw std::invalid_argument(
                "usage: pressure --port <port> [--point <0..459>]");
        std::optional<std::size_t> point;
        if (argc == 5) {
            const std::string value = argv[4];
            std::size_t consumed = 0;
            const auto parsed = std::stoul(value, &consumed);
            if (consumed != value.size() || parsed >= 460)
                throw std::invalid_argument(
                    "--point must be between 0 and 459");
            point = parsed;
        }
        qnbot::GloveConfig glove_config;
        qnbot::SerialConnection connection;
        connection.port = argv[2];
        glove_config.connection = connection;
        qnbot::SdkConfig config;
        config.devices.push_back(glove_config);
        qnbot::Sdk sdk(config);
        auto glove = sdk.glove();
        auto device = glove.device();
        auto pressure = device.pressure();
        const auto subscription =
            pressure.with_config(qnbot::SubscriptionConfig::latest(1.0))
                .subscribe([point](const auto& sample) {
                    print_pressure(sample, point);
                });

        glove.start();
        std::cout << "waiting for pressure (5 s); kPa is an estimate capped at "
                     "~40.01, "
                     "not force; press Ctrl+C to stop"
                  << std::endl;
        const auto deadline =
            std::chrono::steady_clock::now() + std::chrono::seconds(5);
        while (!pressure.latest()) {
            if (std::chrono::steady_clock::now() >= deadline)
                throw std::runtime_error(
                    "No pressure frame within 5 s; check the HMP-MX model, "
                    "serial port and device reporting.");
            glove.update();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        glove.run_forever();
        static_cast<void>(subscription);
        glove.close();
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        return example::report_error(error);
    }
}
