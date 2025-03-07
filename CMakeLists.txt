cmake_minimum_required(VERSION 3.10)
project(pevi C)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)


option(BUILD_TESTS "Build tests" OFF)

# Add third party directory
# Find raylib
include(${CMAKE_CURRENT_SOURCE_DIR}/third_party/raylib.cmake)
add_subdirectory(third_party/linked_ring)

# Include header files
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Add main directory
add_subdirectory(src)


# Add tests if enabled
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
