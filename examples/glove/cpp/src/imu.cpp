#include "example_support.hpp"

#include <cstdlib>
#include <iostream>
#include <iomanip>

void print_pose(const qnbot::Sample<qnbot::GlovePose>& sample) {
    std::cout << std::setprecision(15);
    const auto& hand = sample.value.payload.hand;
    if (!hand) {
        std::cout << "hand absent\n";
    } else {
        std::cout << "hand state=" << (hand->fresh ? "fresh" : "held")
                  << " timestamp_ms=" << hand->timestamp_ms
                  << " received_ms=" << hand->received_monotonic_ms
                  << " fingertips=" << hand->fingertips.size() << "\n";
    }
    const auto& wrist = sample.value.payload.wrist;
    if (!wrist) {
        std::cout << "wrist absent\n";
        return;
    }
    std::cout << "wrist state=" << (wrist->fresh ? "fresh" : "held")
              << " valid=" << wrist->valid
              << " timestamp_ms=" << wrist->timestamp_ms
              << " received_ms=" << wrist->received_monotonic_ms;
    if (wrist->pose) {
        const auto& p = wrist->pose->position;
        std::cout << " position_m=[" << p[0] << "," << p[1] << "," << p[2]
                  << "]";
        const auto& q = wrist->pose->quaternion_xyzw;
        std::cout << " quaternion_xyzw=[" << q[0] << "," << q[1] << "," << q[2]
                  << "," << q[3] << "]";
    } else {
        std::cout << " position_m=none quaternion_xyzw=none";
    }
    std::cout << "\n";
}

int main(int argc, char** argv) {
    try {
        const auto options = example::parse_serial_options(
            argc, argv, example::SerialExample::imu);
        qnbot::Sdk sdk(example::serial_config(options.port, *options.side));
        auto glove = sdk.glove();

        auto device = glove.device();
        auto subscription =
            device.imu()
                .with_config(qnbot::SubscriptionConfig::fifo(512))
                .subscribe([](const qnbot::Sample<qnbot::GloveImu>& sample) {
                    for (const auto& imu : sample.value.payload.imus) {
                        std::cout
                            << "imu id=" << unsigned(imu.id)
                            << " location=" << static_cast<int>(imu.location)
                            << " valid=" << imu.valid << " fresh=" << imu.fresh;
                        if (const auto* raw =
                                std::get_if<qnbot::GloveImuRawCounts>(
                                    &imu.values)) {
                            std::cout << " raw_counts gyro=["
                                      << raw->gyroscope_raw[0] << ","
                                      << raw->gyroscope_raw[1] << ","
                                      << raw->gyroscope_raw[2] << "] accel=["
                                      << raw->accelerometer_raw[0] << ","
                                      << raw->accelerometer_raw[1] << ","
                                      << raw->accelerometer_raw[2] << "]";
                        } else if (const auto* physical = std::get_if<
                                       qnbot::GloveImuPhysicalValues>(
                                       &imu.values)) {
                            if (physical->angular_velocity_rad_s) {
                                const auto& v =
                                    *physical->angular_velocity_rad_s;
                                std::cout << " angular_velocity_rad_s=[" << v[0]
                                          << "," << v[1] << "," << v[2] << "]";
                            } else {
                                std::cout << " angular_velocity_rad_s=none";
                            }
                            if (physical->acceleration_mps2) {
                                const auto& v = *physical->acceleration_mps2;
                                std::cout << " acceleration_mps2=[" << v[0]
                                          << "," << v[1] << "," << v[2] << "]";
                            } else {
                                std::cout << " acceleration_mps2=none";
                            }
                        }
                        std::cout << "\n";
                    }
                });

        auto pose_subscription = device.pose().subscribe(print_pose);

        glove.start();
        std::cout << "running; press Ctrl+C to stop" << std::endl;
        glove.run_forever();
        static_cast<void>(subscription);
        static_cast<void>(pose_subscription);
        glove.close();
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        return example::report_error(error);
    }
}
