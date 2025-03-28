cmake_minimum_required(VERSION 3.10)
project(pevi C)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

option(BUILD_TESTS "Build tests" OFF)

# Add third party directory Find raylib
include(${CMAKE_CURRENT_SOURCE_DIR}/third_party/raylib.cmake)

find_library(LR_TEXT_LIB lr_text REQUIRED)
find_library(LR_LIB lr REQUIRED)
message(STATUS "LR_TEXT_LIB: ${LR_LIB}")
find_path(LR_TEXT_INCLUDE_DIR lr_text.h REQUIRED)
include_directories(${LR_TEXT_INCLUDE_DIR})
if(NOT LR_TEXT_LIB)
  include(FetchContent)
  FetchContent_Declare(
    lr_text
    GIT_REPOSITORY https://github.com/fefa4ka/lr_text.git
    GIT_TAG master)
  FetchContent_MakeAvailable(lr_text) # Get linked_ring source directory
  FetchContent_GetProperties(lr_text SOURCE_DIR LINKED_RING_SOURCE_DIR)
  set(LR_TEXT_LIB lr_text)
endif()

# Include header files
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Add main directory
add_subdirectory(src)

# Copy assets to build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR})

# Add tests if enabled
if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
