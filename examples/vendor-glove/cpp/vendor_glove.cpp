#include <qnbot/sdk.hpp>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>
#include <vector>

int main(int argc, char** argv) {
    try {
        std::string port, side;
        std::uint64_t baudrate = 115200;
        for (int i = 1; i < argc; ++i) {
            const std::string key = argv[i];
            if (key == "--help") {
                std::cout << "vendor_glove --port PORT --side left|right "
                             "[--baudrate 115200]\n";
                return 0;
            }
            if (i + 1 >= argc) throw std::runtime_error("参数缺少值");
            const std::string value = argv[++i];
            if (key == "--port")
                port = value;
            else if (key == "--side")
                side = value;
            else if (key == "--baudrate")
                baudrate = std::stoull(value);
            else
                throw std::runtime_error("未知参数: " + key);
        }
        if (port.empty() || (side != "left" && side != "right") ||
            baudrate == 0 || baudrate > UINT32_MAX)
            throw std::runtime_error("用法: vendor_glove --port PORT --side "
                                     "left|right [--baudrate 115200]");
        std::mutex mutex;
        std::condition_variable wake;
        std::vector<std::uint64_t> sequences;
        qnbot::Sdk sdk({{qnbot::DeviceConfig(qnbot::Value::object(
                            {{"type", "vendor-glove"},
                             {"name", "vendor-glove"},
                             {"side", side},
                             {"connection", qnbot::Value::object(
                                                {{"kind", "serial"},
                                                 {"port", port},
                                                 {"baudrate", baudrate}})}}))},
                        {},
                        std::nullopt,
                        {}});
        const auto sources = sdk.vendor_glove_sources();
        if (sources.size() != 1) throw std::runtime_error("需要一个厂商源");
        const auto& source = sources.front();
        sdk.start();
        for (const std::string operation : {"read_info", "read_parameters"}) {
            const auto reply = sdk.vendor_glove_command(
                source,
                qnbot::Value::object(
                    {{"operation", operation},
                     {"ids",
                      qnbot::Value::array(
                          {qnbot::Value(std::uint64_t{2}),
                           qnbot::Value(std::uint64_t{
                               operation == "read_info" ? 6U : 3U})})}}));
            const auto& result = reply.as_object();
            std::cout << operation
                      << " status=" << result.at("status").as_uint64()
                      << " values={";
            for (const auto& item : result.at("values").as_array()) {
                const auto& tlv = item.as_object();
                std::cout << tlv.at("id").as_uint64() << ":[";
                for (const auto& byte : tlv.at("value").as_array())
                    std::cout << byte.as_uint64() << ',';
                std::cout << "] ";
            }
            std::cout << "}\n";
        }
        auto subscription = sdk.vendor_glove_raw_sensors(source).subscribe(
            [&](const qnbot::Sample<qnbot::Value>& sample) {
                const auto& value = sample.value.as_object();
                std::lock_guard<std::mutex> lock(mutex);
                sequences.push_back(sample.sequence);
                std::cout << "sample=" << sample.sequence << " wire_sequence="
                          << value.at("wire_sequence").as_uint64()
                          << " bitmap=" << value.at("bitmap").as_uint64()
                          << " timestamp_ms=";
                const auto& timestamp = value.at("timestamp_ms");
                if (timestamp.storage().index() == 0)
                    std::cout << "null";
                else
                    std::cout << timestamp.as_uint64();
                std::cout << " blocks={";
                for (const auto& block : value.at("blocks").as_object())
                    std::cout << block.first << ':'
                              << block.second.as_array().size() << ' ';
                std::cout << "}\n";
                wake.notify_all();
            });
        {
            std::unique_lock<std::mutex> lock(mutex);
            if (!wake.wait_for(lock, std::chrono::seconds(10),
                               [&] { return sequences.size() >= 10; }))
                throw std::runtime_error(
                    "10 秒内未收到 10 帧；检查设备模式、订阅表和连接");
            for (std::size_t i = 1; i < sequences.size(); ++i)
                if (sequences[i] <= sequences[i - 1])
                    throw std::runtime_error("SDK 样本序号未前进");
        }
        sdk.stop();
        subscription.unsubscribe();
        sdk.close();
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
