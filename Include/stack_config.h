/**
 * @file stack_config.h
 *
 * @brief Stack configuration parameters
 *
 * $Id: stack_config.h 37468 2015-06-03 09:04:25Z sschneid $
 *
 * @author    Atmel Corporation: http://www.atmel.com
 * @author    Support email: avr@atmel.com
 */
/*
 * Copyright (c) 2009, Atmel Corporation All rights reserved.
 *
 * Licensed under Atmel's Limited License Agreement --> EULA.txt
 */

/* Prevent double inclusion */
#ifndef STACK_CONFIG_H
#define STACK_CONFIG_H

#include "stack_layers.h"
#if (HIGHEST_STACK_LAYER == PAL)
#   include "pal.h"
#elif (HIGHEST_STACK_LAYER == TAL)
#   include "tal.h"
#elif (HIGHEST_STACK_LAYER == MAC)
#   include "mac_api.h"
#else
#   error "Undefined highest stack layer"
#endif


/* First timer ID from applications */
#define APP_FIRST_TIMER_ID                  (0x1000)

/*
 * Value to indicate end of timer in the array or queue
 * Do NOT change this value.
 */
#define NO_TIMER                            (0xFF)
#if (NO_TIMER) > 0xFF
#   error"NO_TIMER must not exceed size of uint8_t"
#endif

/*
 * Maximum number of timer (actually running in parallel)
 * supported within the stack by an application.
 * Note: The actual number of overall timers may by far be larger than this,
 * because usually only a certain number of timers are actually running at the same
 * time.
 */
#define MAX_NO_OF_PARALLEL_RUNNING_TIMERS   (8)
#if (MAX_NO_OF_PARALLEL_RUNNING_TIMERS) >= NO_TIMER
#   error"MAX_NO_OF_PARALLEL_RUNNING_TIMERS must be smaller than NO_TIMER"
#endif

#endif /* STACK_CONFIG_H */
/* EOF */
