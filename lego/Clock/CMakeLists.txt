project(Clock VERSION 1.0
    DESCRIPTION "eer calendar, date and time"
    LANGUAGES C)

option(CLOCK_DATE "Timestamp to calender convertion" ON)

if(${CLOCK_DATE})
    add_definitions(-D_Clock_date)
endif()

set(TEST_HAL_LIBS gpio timer)

include(../component.cmake)
