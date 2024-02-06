#pragma once

#include <hal.h>
#include "log.h"
#ifdef PROFILING
    #include "profiler.h"
#endif

const char *int_to_binary_str(int x, int N_bits);

#define test(...) test_program({ gpio_init(); }, { free_pins(); }, __VA_ARGS__)

#define eer_hw_isr_enable()                                                        \
    {                                                                          \
    }
#define eer_hw_isr_disable()                                                       \
    {                                                                          \
    }
#define debug(port, pin)                                                       \
    ({                                                                         \
    })

#define eer_hw_pin(port, pin)                                                  \
    {                                                                          \
    }


extern eer_timer_handler_t  eer_hw_timer;

typedef uint64_t timer_size_t;
#define TIMER_MAX UINT64_MAX
