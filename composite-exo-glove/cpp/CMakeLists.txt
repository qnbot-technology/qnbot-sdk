cmake_minimum_required(VERSION 3.16)
project(qnbot_composite_exo_glove_cpp_examples LANGUAGES CXX)

set(CMAKE_CXX_EXTENSIONS OFF)

find_package(qnbot_exo CONFIG REQUIRED)
find_package(qnbot_glove CONFIG REQUIRED)
find_package(Threads REQUIRED)

add_library(qnbot_composite_exo_glove_example_options INTERFACE)
target_compile_features(qnbot_composite_exo_glove_example_options INTERFACE cxx_std_17)
target_link_libraries(qnbot_composite_exo_glove_example_options INTERFACE
  qnbot::exo qnbot::glove Threads::Threads)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  target_compile_options(qnbot_composite_exo_glove_example_options INTERFACE
    -Wall -Wextra -Werror -Wpedantic)
endif()

function(add_qnbot_composite_exo_glove_example target)
  add_executable(${target} src/${target}.cpp)
  target_link_libraries(${target} PRIVATE
    qnbot_composite_exo_glove_example_options)
endfunction()

add_qnbot_composite_exo_glove_example(discover_exo_glove)
add_qnbot_composite_exo_glove_example(quick_start)
add_qnbot_composite_exo_glove_example(device_info)
add_qnbot_composite_exo_glove_example(telemetry)
add_qnbot_composite_exo_glove_example(imu)
add_qnbot_composite_exo_glove_example(exo_haptics)
add_qnbot_composite_exo_glove_example(lifecycle)
