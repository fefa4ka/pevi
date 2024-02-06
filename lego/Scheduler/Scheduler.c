#include "Scheduler.h"
#include <eers.h>

/* Priority queue navigation */
#define LEFT(x)   (2 * (x) + 1)
#define RIGHT(x)  (2 * (x) + 2)
#define PARENT(x) ((x) / 2)

result_t event_callback(void *instance, void *argument);
int  event_compare(unsigned int now, struct Scheduler_event *a,
                   struct Scheduler_event *b);
void event_heapify(struct Scheduler_queue *scheduler, unsigned int now,
                   unsigned char idx);
void event_prioritify(struct Scheduler_queue *queue, unsigned int now,
                      struct Scheduler_event *new_event);

/**
 * \brief    Add event to queue
 */
bool Scheduler_enqueue(Scheduler_t *self, unsigned int timeout_us,
                       struct eer_callback *callback)
{
    struct Scheduler_queue *queue = &self->state.queue;
    eer_timer_handler_t *   timer = self->props.timer;
    unsigned int            now;

    timer->get(&now);

    if (queue->size == queue->capacity || timeout_us == 0) {
        /* Not enought space in queue */
        return false;
    }

    struct Scheduler_event new_event = {timeout_us, now};
    new_event.callback               = *callback;

    event_prioritify(queue, now, &new_event);

    return true;
}

/**
 * \brief    Add event to queue
 */
void event_prioritify(struct Scheduler_queue *queue, unsigned int now,
                      struct Scheduler_event *new_event)
{
    unsigned char           index  = queue->size;
    struct Scheduler_event *events = queue->events;

    queue->events[index] = *new_event;
    queue->size += 1;

    /* Sorting */
    struct Scheduler_event swap = {0};
    while (index > 0
           && event_compare(now, &events[index], &events[PARENT(index)]) > 0) {
        swap                  = events[index];
        events[index]         = events[PARENT(index)];
        events[PARENT(index)] = swap;
        index                 = PARENT(index);
    }
}

/**
 * \brief    Fire callback and turn off timer
 */
result_t event_callback(void *instance, void *argument)
{
    Scheduler_t *self = instance;
    self->props.timer->isr.disable(0);

    struct Scheduler_event triggered_event = {0};
    triggered_event                        = self->state.next_event;

    triggered_event.callback.method(triggered_event.callback.argument, instance);
    struct Scheduler_event null_event = {0};
    self->state.next_event            = null_event;

    return OK;
}

/**
 * \brief    If result is positive event A is close than B
 */
int event_compare(unsigned int now, struct Scheduler_event *a,
                  struct Scheduler_event *b)
{
    int          a_passed  = now - a->created;
    unsigned int a_timeout = a->timeout_us << 1;
    if (a_passed < 0) {
        a_passed = 65535 - a->created + now;
    }

    int          b_passed  = now - b->created;
    unsigned int b_timeout = b->timeout_us << 1;
    if (b_passed < 0) {
        b_passed = 65535 - b->created + now;
    }

    int a_remain = a_passed - a_timeout;
    int b_remain = b_passed - b_timeout;

    /*
    a->created = now;
//    a_passed = a_passed >> 1;
  //  b_passed = b_passed >> 1;
    if (a_timeout > a_passed) {
        a->timeout_us -= a_passed >> 1;
    } else {
        log_pin (B, 5);
        a->timeout_us = 0;
    }

    b->created = now;
    if (b_timeout > b_passed) {
        b->timeout_us -= b_passed >> 1;
    } else {
        log_pin (B, 0);
        b->timeout_us = 0;
    }
    */

    return a_remain - b_remain;
}

/**
 * \brief    Reorder queue with event priority
 */
void event_heapify(struct Scheduler_queue *scheduler, unsigned int now,
                   unsigned char idx)
{
    struct Scheduler_event  swap   = {0};
    struct Scheduler_event *events = scheduler->events;

    unsigned char l_idx, r_idx, lrg_idx;

    l_idx = LEFT(idx);
    r_idx = RIGHT(idx);

    /* Left child exists, compare left child with its parent */
    if (l_idx < scheduler->size
        && event_compare(now, &events[l_idx], &events[idx]) > 0) {
        lrg_idx = l_idx;
    } else {
        lrg_idx = idx;
    }

    /* Right child exists, compare right child with the largest element */
    if (r_idx < scheduler->size
        && event_compare(now, &events[r_idx], &events[lrg_idx]) > 0) {
        lrg_idx = r_idx;
    }

    /* At this point largest element was determined */
    if (lrg_idx != idx) {
        /* Swap between the index at the largest element */
        swap            = events[lrg_idx];
        events[lrg_idx] = events[idx];
        events[idx]     = swap;
        /* Heapify again */
        event_heapify(scheduler, now, lrg_idx);
    }
}

/**
 * \brief    Get closest event
 */
bool Scheduler_dequeue(Scheduler_t *self, struct Scheduler_event *dequed_event)
{
    struct Scheduler_queue *queue      = &self->state.queue;
    eer_timer_handler_t *   timer      = self->props.timer;
    struct Scheduler_event *events     = queue->events;
    unsigned int            last_index = queue->size - 1;
    timer_size_t moment;

    /* Empty queue */
    struct Scheduler_event null_event = {0};
    if (queue->size == 0) {
        *dequed_event = null_event;
        return false;
    }

    struct Scheduler_event closest_event = {0};
    closest_event                        = events[0];

    events[0] = events[last_index];
    queue->size -= 1;

    if (queue->size == 0) {
        /* Now queue empty */
        events[0] = null_event;

        *dequed_event = closest_event;
        return true;
    } else {
        events[last_index] = null_event;
    }

    /* Queue reordering because last slot swapped */
    if (queue->size > 1) {
        timer->get(&moment);
        event_heapify(queue, moment, 0);
    }

    *dequed_event = closest_event;
    return true;
}

/**
 * \brief     Clean pointers
 */
WILL_MOUNT(Scheduler)
{
    /* comment if used with timer */
    // props->timer->init(ptr);
    struct Scheduler_event null_event = {0};
    state->next_event                 = null_event;
}

/**
 * \brief     Update if no sheduled event
 *            or new closest event available
 */
SHOULD_UPDATE(Scheduler)
{
    timer_size_t now;

    // If queue empty, there are nothing to do
    if (state->queue.size == 0) {
        return false;
    }

    struct Scheduler_event *closest_event = &state->queue.events[0];

    // If current closest timeout as next event timeout, everything is setup
    //    if(closest_event->timeout_us != 0
    //            && closest_event->timeout_us == state->next_event.timeout_us)
    //            {

    /* If next event closest that in queue */
    props->timer->get(&now);
    if (closest_event->callback.method && state->next_event.callback.method
        && event_compare(now, &state->next_event,
                         closest_event)) {
        return false;
    }

    return true;
}

/**
 * \brief     Get closest event
 */
WILL_UPDATE(Scheduler)
{
    timer_size_t now;
    props->timer->get(&now);

    if (state->next_event.callback.method) {
        /* Event allready scheduled, back it to queue */
        props->timer->isr.disable(0);
        event_prioritify(&state->queue, now,
                         &state->next_event);
    }

    /* Get new event with highest priority */
    Scheduler_dequeue((Scheduler_t *)self, &state->next_event);
}

/**
 * \brief    Fire or schedule event callback
 */
RELEASE(Scheduler)
{
    if (!state->next_event.callback.method) {
        /* No events */
        return;
    }

    timer_size_t now;
    timer_size_t moment;
    int          passed;
    unsigned int timeout        = props->timer->us_to_ticks(state->next_event.timeout_us);
    unsigned int scheduled_tick;

    props->timer->get(&now);
     passed         = now - state->next_event.created;
    scheduled_tick = now + timeout;
    /* Handle timer overflow */
    if (passed < 0) {
        passed = TIMER_MAX - state->next_event.created + now;
    }

    scheduled_tick -= passed;

    if (passed > timeout
        || (scheduled_tick > now && *(timer_size_t *)props->timer->get(&moment) > scheduled_tick)
        || (now > scheduled_tick && scheduled_tick > *(timer_size_t *)props->timer->get(&moment))) {
        /* Scheduled event allready should happen */
        event_callback(self, 0);
    } else {
        /* Schedule callback by timer, probably interrupt will be used */
        struct eer_timer_isr isr_settings = {scheduled_tick, TIMER_EVENT_COMPARE};
        struct eer_callback  isr_routine  = {event_callback, self};
        /* isr_routine in stack, could not be used with ISR component */
        props->timer->isr.enable(&isr_settings, &isr_routine);
    }
}

DID_MOUNT_SKIP(Scheduler);
DID_UPDATE_SKIP(Scheduler);
