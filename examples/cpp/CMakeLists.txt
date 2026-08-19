cmake_minimum_required(VERSION 3.16)
project(qnbot_cpp_examples LANGUAGES CXX)

set(CMAKE_CXX_EXTENSIONS OFF)

find_package(qnbot_glove CONFIG REQUIRED)
find_package(Threads REQUIRED)

add_library(qnbot_cpp17_examples INTERFACE)
target_compile_features(qnbot_cpp17_examples INTERFACE cxx_std_17)
target_link_libraries(qnbot_cpp17_examples INTERFACE qnbot::glove Threads::Threads)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  target_compile_options(qnbot_cpp17_examples INTERFACE
    -Wall -Wextra -Werror -Wpedantic)
endif()

function(add_qnbot_cpp17_example target)
  add_executable(${target} examples/${target}.cpp)
  target_link_libraries(${target} PRIVATE qnbot_cpp17_examples)
endfunction()

add_qnbot_cpp17_example(external_input)
add_qnbot_cpp17_example(discover_gloves)
add_qnbot_cpp17_example(quick_start)
add_qnbot_cpp17_example(foreground_runtime)
add_qnbot_cpp17_example(background_runtime)
add_qnbot_cpp17_example(manual_runtime)
add_qnbot_cpp17_example(haptics)
add_qnbot_cpp17_example(device_authorization)

add_library(qnbot_cpp20_examples INTERFACE)
target_compile_features(qnbot_cpp20_examples INTERFACE cxx_std_20)
target_link_libraries(qnbot_cpp20_examples INTERFACE
  qnbot::glove qnbot::core_coro Threads::Threads)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  target_compile_options(qnbot_cpp20_examples INTERFACE
    -Wall -Wextra -Werror -Wpedantic)
endif()

add_executable(coroutine_next examples/coroutine_next.cpp)
target_link_libraries(coroutine_next PRIVATE qnbot_cpp20_examples)
