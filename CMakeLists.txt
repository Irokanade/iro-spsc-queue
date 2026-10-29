cmake_minimum_required(VERSION 4.2)
project(iro_spsc_queue)

set(CMAKE_CXX_STANDARD 20)

add_library(iro_spsc INTERFACE)
target_include_directories(iro_spsc INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
target_compile_features(iro_spsc INTERFACE cxx_std_20)

add_executable(iro_spsc_queue main.cpp)
target_link_libraries(iro_spsc_queue PRIVATE iro_spsc)
