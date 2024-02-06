#include "Clock.h"

#include <eer_test.h>
#include <unit.h>


/* Datetime couting */
Clock(clk, &hw(timer), 0);
struct Clock_calendar now;

test(ticks) {
    clk.props.calendar = &now;

    ignite(clk);

    terminate;
}


result_t ticks()
{
    test_assert(clk.state.time.timestamp == 0, "Clock should be 0 at start, but %d", clk.state.ms);
    usleep(1.1 * 1e6);
    test_assert(clk.state.time.timestamp == 1, "Clock should be 1, but %d (clk_ms = %d)", clk.state.time.timestamp, clk.state.ms);
    usleep(1.1 * 1e6);
    test_assert(clk.state.time.timestamp == 2, "Clock should be 2, but %d (clk_ms = %d)", clk.state.time.timestamp, clk.state.ms);
    usleep(1.1 * 1e6);
    test_assert(clk.state.time.timestamp == 3, "Clock should be 3, but %d (clk_ms = %d)", clk.state.time.timestamp, clk.state.ms);
    usleep(1.1 * 1e6);
    test_assert(clk.state.time.timestamp == 4, "Clock should be 4, but %d (clk_ms = %d)", clk.state.time.timestamp, clk.state.ms);
    usleep(1.1 * 1e6);
    test_assert(clk.state.time.timestamp == 5, "Clock should be 5, but %d (clk_ms = %d)", clk.state.time.timestamp, clk.state.ms);

    test_assert(now.year == 1970, "Year should be 1970, but %d", now.year);


    return OK;
}


