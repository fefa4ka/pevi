#pragma once

#include <eer.h>

/*
 *  ┌─────┐ Scheduler_enqueue() ┌─────┐  get closest  ┌─────────────────┐
 *  │event├────────────────────►│QUEUE├──────────────►│Fire or set Timer│
 *  └─────┘                     └─────┘               └─────────────────┘
 *
 *  ┌───────────◄──┐      ┌────────────┐       ┌─────────┐
 *  │ ShouldUpdate ├─────►│ WillUpdate ├──────►│ Release │
 *  └──────────────┘      └────────────┘       └─────────┘
 *   if in queue           Prioritity queue     Fire event
 *   new closest event     and clean timer      or set timer
 */

#define Scheduler(instance, scheduler_capacity, props)                         \
    struct Scheduler_event instance##_events[scheduler_capacity];              \
    eer_define(Scheduler, instance, _(props),                                  \
               _({                                                             \
                   .queue = {.events   = instance##_events,                    \
                             .capacity = scheduler_capacity},                  \
               }))

/* Define with Timer handler */
#define Scheduler_timer_handler(instance)                                      \
    extern eer_timer_handler_t instance##_timer_handler;                       \
    void                       instance##_timer_init(void *args)               \
    {                                                                          \
        if (!instance##_timer_handler.get) {                                   \
            instance##_timer_handler.get = instance.props.timer->get;          \
            instance##_timer_handler.isr.disable                               \
                = instance.props.timer->isr.disable;                           \
        }                                                                      \
        instance.props.timer->init(args);                                      \
    }                                                                          \
    void instance##_timer_isr_enable(void *args_ptr, eer_callback_t *callback) \
    {                                                                          \
        struct eer_timer_isr *args = (struct eer_timer_isr *)args_ptr;         \
        Scheduler_enqueue(&instance, args->ticks, callback);                   \
    }                                                                          \
    eer_timer_handler_t instance##_timer_handler                               \
        = {.init = instance##_timer_init,                                      \
           .isr  = {.enable = instance##_timer_isr_enable}}

///
/// \brief Scheduled event with callback and args
///
struct Scheduler_event {
    unsigned int        timeout_us;
    unsigned int        created;
    struct eer_callback callback;
};

///
/// \brief Priority queue
///
struct Scheduler_queue {
    unsigned char           size;
    unsigned char           capacity;
    struct Scheduler_event *events;
};

typedef struct {
    eer_timer_handler_t *timer; /* Use timer callback on tick comparasion */

} Scheduler_props_t;

typedef struct {
    bool                   scheduled;  /* Is next event timer set */
    struct Scheduler_event next_event; /* Scheduled event */
    struct Scheduler_queue queue;      /* Priority queue */
} Scheduler_state_t;

eer_header(Scheduler);

bool Scheduler_enqueue(Scheduler_t *, unsigned int timeout_us,
                       struct eer_callback *);
bool Scheduler_dequeue(Scheduler_t *, struct Scheduler_event *);
