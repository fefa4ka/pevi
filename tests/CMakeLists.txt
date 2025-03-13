cmake_minimum_required(VERSION 3.10)

# Function to add a test with dependencies
function(add_pevi_test TEST_NAME TEST_SOURCE)
    # Create the test executable
    add_executable(${TEST_NAME} ${TEST_SOURCE} ${ARGN})
    
    # Link with necessary libraries
    target_link_libraries(${TEST_NAME}
        PRIVATE
        raylib
        lr
    )
    
    # Include directories
    target_include_directories(${TEST_NAME}
        PRIVATE
        ${CMAKE_SOURCE_DIR}/include
    )
    
    # Add test to CTest
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endfunction()

# Add memory test
add_pevi_test(memory_test 
    memory_test.c 
    ${CMAKE_SOURCE_DIR}/src/memory.c 
    ${CMAKE_SOURCE_DIR}/src/logger.c 
    ${CMAKE_SOURCE_DIR}/src/error.c
)
