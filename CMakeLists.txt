cmake_minimum_required(VERSION 4.2)
project(iro_spsc_queue LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

add_library(iro_spsc INTERFACE)
target_sources(iro_spsc INTERFACE
    FILE_SET HEADERS
    BASE_DIRS include
    FILES include/iro/lockfree_spsc.hpp)
target_compile_features(iro_spsc INTERFACE cxx_std_20)
