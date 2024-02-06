# Scheduler

The Scheduler component is used to schedule events to be executed at a specific time in the future. It uses a timer interrupt to trigger the events. The component has a priority queue to store the scheduled events, and the event with the nearest execution time is always at the front of the queue.

To schedule an event, the `Scheduler_enqueue()` function is used, which adds the event to the priority queue and reconfigures the timer interrupt if necessary. When the timer interrupt is triggered, the event at the front of the queue is removed from the queue and executed.

The Scheduler component can be used to schedule events in a wide range of applications, such as timers, alarms, and periodic tasks. It can be useful in embedded systems where precise timing and event scheduling are required.
