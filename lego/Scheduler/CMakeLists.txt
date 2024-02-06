project(Scheduler VERSION 1.0
    DESCRIPTION "eer priority queue scheduler based on timeout"
    LANGUAGES C)

set(LIBS Clock)
set(TEST_HAL_LIBS gpio timer)
include(../component.cmake)
