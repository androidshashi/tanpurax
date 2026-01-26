# android/app/src/main/cpp/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

project(tanpura)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -ffast-math -Wall")

# Add Oboe library
set(OBOE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../oboe)
add_subdirectory(${OBOE_DIR} ./oboe)
include_directories(${OBOE_DIR}/include)

# Add source files
add_library(
    tanpura
    SHARED
    TanpuraString.cpp
    TanpuraEngine.cpp
    tanpura_jni.cpp
)

find_library(log-lib log)
find_library(android-lib android)

target_link_libraries(
    tanpura
    oboe
    ${log-lib}
    ${android-lib}
)