#pragma once

#include <eer.h>

#define Clock(instance, timer_handler, timestamp)                              \
    eer_define(Clock, instance, _({.timer = timer_handler}),                   \
               _({.time = {timestamp}}))
#define Clock_time(instance) eer_state(Clock, instance, time)
#define Clock_set(instance, timestamp)                                         \
    {                                                                          \
        Time_state_t *state   = instance.state;                                \
        state->time.timestamp = timestamp;                                     \
    }
#define Clock_get_year(instance)                                               \
    Clock_time(instance).timestamp / 31536000 + 1970


struct Clock_time {
    uint32_t timestamp; /* Unix timestamp in seconds */
    uint16_t ms;        /* Tick counter in ms */
    uint32_t us;        /* Tick counter in us */
    uint16_t step_us;   /* Tick length in us */
};

struct Clock_calendar {
    uint8_t  sec;    /* seconds after the minute [0-60] */
    uint8_t  min;    /* minutes after the hour [0-59] */
    uint8_t  hour;   /* hours since midnight [0-23] */
    uint8_t  mday;   /* day of the month [1-31] */
    uint8_t  mon;    /* months since January [0-11] */
    uint16_t year;   /* years since 1900 */
    uint8_t  wday;   /* days since Sunday [0-6] */
    uint16_t yday;   /* days since January 1 [0-365] */
    uint8_t  isdst;  /* Daylight Savings Time flag */
    uint16_t gmtoff; /* offset from CUT in seconds */
    uint8_t *zone;   /* timezone abbreviation */
};

typedef struct {
    eer_timer_handler_t   *timer;
    struct Clock_calendar *calendar;
    void (*onSecond)(eer_t *instance);
} Clock_props_t;

typedef struct {
    struct Clock_time time;

    uint16_t     ms;     /* Collect ms for seconds tick */
    uint16_t     us;     /* Collect us for ms tick */
    timer_size_t tick;   /* Last timer value */
    timer_size_t passed; /* Passed from last check in timer ticks */
} Clock_state_t;

eer_header(Clock);
#ifdef _Clock_date
void Clock_date(unsigned long seconds, struct Clock_calendar *tm);
#endif
