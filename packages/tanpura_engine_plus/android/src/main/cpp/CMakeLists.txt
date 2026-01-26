cmake_minimum_required(VERSION 3.18)
project(tanpura_native)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fetch Oboe for low-latency audio
include(FetchContent)
FetchContent_Declare(
    oboe
    GIT_REPOSITORY https://github.com/google/oboe.git
    GIT_TAG 1.8.0
)
FetchContent_MakeAvailable(oboe)

# Build native library
add_library(
    tanpura_native
    SHARED
    tanpura_jni.cpp
    tanpura_engine.cpp
)

# Include paths
target_include_directories(
    tanpura_native
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${oboe_SOURCE_DIR}/include
)

# Link libraries
target_link_libraries(
    tanpura_native
    oboe
    android
    log
)

# Optimization flags
target_compile_options(
    tanpura_native
    PRIVATE
    -Wall
    -Wextra
    -O3
    -ffast-math
)
