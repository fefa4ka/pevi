project(font VERSION 1.0
    DESCRIPTION "pevi fonts"
    LANGUAGES C)

SET(LEGO core)
include(../component.cmake)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets/font.ttf 
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets/sdf.fs 
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
