project(Serial VERSION 1.0
    DESCRIPTION "eer serial transiever"
    LANGUAGES C)

set(LIBS lr)

set(TEST_HAL_LIBS gpio uart)

include(../component.cmake)
