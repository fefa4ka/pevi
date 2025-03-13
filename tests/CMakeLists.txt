cmake_minimum_required(VERSION 3.10)

# Add test executable
add_executable(memory_test memory_test.c)

# Link with necessary libraries
target_link_libraries(memory_test
    PRIVATE
    raylib
    lr
)

# Include directories
target_include_directories(memory_test
    PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# Add test to CTest
add_test(NAME MemoryTest COMMAND memory_test)
