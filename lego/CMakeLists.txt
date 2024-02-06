cmake_minimum_required(VERSION 3.1)

project(eeReactLego VERSION 1.0
    DESCRIPTION "Embedded Event-driven React Component Library"
    LANGUAGES C)

set(EER_PATH "/eer")
include(eer/options.cmake)

add_subdirectory(third_party/linked_ring)
add_subdirectory(eer)

# Abstract
add_subdirectory(Clock)
add_subdirectory(Scheduler)
add_subdirectory(Menu)

# 3D
add_subdirectory(Window)
add_subdirectory(Camera)
add_subdirectory(Symbol)
add_subdirectory(Text)
