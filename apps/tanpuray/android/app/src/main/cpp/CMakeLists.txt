# android/app/src/main/cpp/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

project(tanpura)

# C++17 for better performance
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -ffast-math")

add_library(
    tanpura
    SHARED
    tanpura_jni.cpp
)

find_library(log-lib log)

target_link_libraries(
    tanpura
    ${log-lib}
)