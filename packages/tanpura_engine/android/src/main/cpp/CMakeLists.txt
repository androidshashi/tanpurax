cmake_minimum_required(VERSION 3.18)

project(tanpura_engine)

# ----------------------------------------
# Add Oboe library
# ----------------------------------------
add_subdirectory(oboe)

# ----------------------------------------
# Build our native library
# ----------------------------------------
add_library(
        tanpura_engine
        SHARED
        tanpura_jni.cpp
        audio_engine.cpp
)

# ----------------------------------------
# Include paths
# ----------------------------------------
target_include_directories(
        tanpura_engine
        PRIVATE
        oboe/include
)

# ----------------------------------------
# Link libraries
# ----------------------------------------
target_link_libraries(
        tanpura_engine
        oboe
        android
        log
)
