cmake_minimum_required(VERSION 3.23) # FILE_SET needs 3.23
project(iro_spsc_queue VERSION 0.1.0 DESCRIPTION "Header-only lock-free SPSC queue" LANGUAGES CXX)

# Header-only: listing the header in a file set makes it belong to the target (IDEs rely
# on this) and adds include/ to the include path of anything that links the library.
add_library(iro_spsc INTERFACE)
add_library(iro::spsc ALIAS iro_spsc)
target_sources(iro_spsc INTERFACE
    FILE_SET HEADERS
    BASE_DIRS include
    FILES include/iro/lockfree/spsc_queue.hpp)
target_compile_features(iro_spsc INTERFACE cxx_std_20)
