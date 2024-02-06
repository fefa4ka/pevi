# Clock

`Clock` component provides an abstraction layer for timekeeping in applications. It allows for the configuration of a timer and provides a time struct `Clock_time` with timestamp, ms, and us fields, as well as a calendar struct `Clock_calendar` with fields for the current date and time. The component also allows for setting callbacks for events such as the passing of a second or minutes.

## Usage

Declaration and usage of the `Clock` component with a hardware timer:

```c
Clock(clk, &hw(timer), TIMESTAMP);

/* Inside event-loop
 * Just initialize and let live
 */
loop(clk) { ... }
ignite(clk);

```

In this example, the `clk` component is initialized with a hardware timer and a starting timestamp of `TIMESTAMP`. The component's `state.time` struct can be accessed using the `Clock_time` macro and the current time can be set using the `Clock_set` macro.

The Clock component is implemented using a timer handler, which is a function provided by the user that is called at regular intervals. The timer handler increments a timestamp and counters for milliseconds and microseconds. It also calls a callback function (`on.second`) whenever a second has passed. The Clock component also provides a function (`Clock_date`) that converts a timestamp into a calendar date.

## Applications

The `Clock` component can be used in various applications, for example:

-   Timestamping - to provide the timestamp for the log entry.
-   Real-time Clock - to provide an RTC functionality in a microcontroller-based system.
-   Timekeeping - to keep track of time in a system, for example, in a stopwatch application or a timer application.
-   Scheduling - to schedule tasks to be performed at specific times: second, minute, hour, day
-   Data Synchronization - to synchronize data between different devices or systems by providing a common time reference.
-   Calendar - to provide the date and time information for a calendar application.
-   [Button](../Button) - to debouncing
