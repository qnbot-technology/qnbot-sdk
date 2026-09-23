cmake_minimum_required(VERSION 3.16)
project(qnbot_exo_cpp_examples LANGUAGES CXX)

set(CMAKE_CXX_EXTENSIONS OFF)

find_package(qnbot_exo CONFIG REQUIRED)
find_package(Threads REQUIRED)

add_library(qnbot_exo_example_options INTERFACE)
target_compile_features(qnbot_exo_example_options INTERFACE cxx_std_17)
target_link_libraries(qnbot_exo_example_options INTERFACE
  qnbot::exo Threads::Threads)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  target_compile_options(qnbot_exo_example_options INTERFACE
    -Wall -Wextra -Werror -Wpedantic)
endif()

function(add_qnbot_exo_example target)
  add_executable(${target} src/${target}.cpp)
  target_link_libraries(${target} PRIVATE qnbot_exo_example_options)
endfunction()

add_qnbot_exo_example(discover_exos)
add_qnbot_exo_example(quick_start)
add_qnbot_exo_example(device_info)
add_qnbot_exo_example(telemetry)
add_qnbot_exo_example(imu)
add_qnbot_exo_example(handset)
add_qnbot_exo_example(haptics)
add_qnbot_exo_example(device_lifecycle)
