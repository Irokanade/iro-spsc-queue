cmake_minimum_required(VERSION 4.2)
project(iro_spsc_queue LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

add_library(iro_spsc INTERFACE)
target_include_directories(iro_spsc INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
target_compile_features(iro_spsc INTERFACE cxx_std_20)
