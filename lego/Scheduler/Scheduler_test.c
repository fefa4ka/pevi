#include <Clock.h>
#include <Scheduler.h>

#include <eers.h>
#include <unit.h>

#define TIMEOUT_5 1200
#define TIMEOUT_7 2000
#define TIMEOUT_6 1500
#define TIMEOUT_8 5500
#define TIMEOUT_1 500
#define TIMEOUT_2 700
#define TIMEOUT_3 900
#define TIMEOUT_4 1000

/* Timer and callback scheduler */
Clock(clk, &hw(timer), TIMESTAMP);
Scheduler(scheduler, 15, _({.timer = &hw(timer)}));


int trigger_count = 0;
eer_result_t scheduler_trigger(void *arg, void *trigger) {
    int value = *(int *)arg;

    trigger_count++;

    if(trigger_count == 1)
        test_assert(value == TIMEOUT_1, "Scheduled event #%d - %d != %d", trigger_count, *(int *)arg, TIMEOUT_1);
    if(trigger_count == 2)
        test_assert(value == TIMEOUT_2, "Scheduled event #%d - %d != %d", trigger_count, *(int *)arg, TIMEOUT_2);
    if(trigger_count == 3)
        test_assert(value == TIMEOUT_3, "Scheduled event #%d - %d != %d", trigger_count, *(int *)arg, TIMEOUT_3);
    if(trigger_count == 4)
        test_assert(value == TIMEOUT_4, "Scheduled event #%d - %d != %d", trigger_count, *(int *)arg, TIMEOUT_4);
    if(trigger_count == 5)
        test_assert(value == TIMEOUT_5, "Scheduled event #%d - %d != %d", trigger_count, *(int *)arg, TIMEOUT_5);
    if(trigger_count == 6)
        test_assert(value == TIMEOUT_6, "Scheduled event #%d - %d != %d", trigger_count, *(int *)arg, TIMEOUT_6);
    if(trigger_count == 7)
        test_assert(value == TIMEOUT_7, "Scheduled event #%d - %d != %d", trigger_count, *(int *)arg, TIMEOUT_7);
    if(trigger_count == 8)
        test_assert(value == TIMEOUT_8, "Scheduled event #%d - %d != %d", trigger_count, *(int *)arg, TIMEOUT_8);

    return OK;
}


int count = 0;
test(queue_push_pop) {
    loop(clk, scheduler) {
        count++;
    }
}

result_t queue_push_pop() {
    struct eer_callback scheduler_callback = { scheduler_trigger };
    int a = TIMEOUT_1;
    int b = TIMEOUT_4;
    int c = TIMEOUT_3;
    int d = TIMEOUT_2;
    int e = TIMEOUT_8;
    int f = TIMEOUT_6;
    int g = TIMEOUT_5;
    int h = TIMEOUT_7;

    scheduler_callback.argument = &a;
    Scheduler_enqueue(&scheduler, a, &scheduler_callback);
    scheduler_callback.argument = &b;
    Scheduler_enqueue(&scheduler, b, &scheduler_callback);
    scheduler_callback.argument = &c;
    Scheduler_enqueue(&scheduler, c, &scheduler_callback);
    scheduler_callback.argument = &d;
    Scheduler_enqueue(&scheduler, d, &scheduler_callback);
    scheduler_callback.argument = &e;
    Scheduler_enqueue(&scheduler, e, &scheduler_callback);
    scheduler_callback.argument = &f;
    Scheduler_enqueue(&scheduler, f, &scheduler_callback);
    scheduler_callback.argument = &g;
    Scheduler_enqueue(&scheduler, g, &scheduler_callback);
    scheduler_callback.argument = &h;
    Scheduler_enqueue(&scheduler, h, &scheduler_callback);

    sleep(2);

    return OK;
}
