/**
 * @file pal_timer.h
 *
 * @brief Generic PAL timer internal functions prototypes
 *
 * $Id: pal_timer.h 37446 2015-05-29 08:25:07Z sschneid $
 *
 * @author    Atmel Corporation: http://www.atmel.com
 * @author    Support email: avr@atmel.com
 */
/*
 * Copyright (c) 2015, Atmel Corporation All rights reserved.
 *
 * Licensed under Atmel's Limited License Agreement --> EULA.txt
 */


/* Prevent double inclusion */
#ifndef PAL_TIMER_H
#define PAL_TIMER_H

/* === Includes ============================================================= */

#include "pal_types.h"
#if (PAL_GENERIC_TYPE == MEGA_RF) || (PAL_GENERIC_TYPE == SAM4) || (PAL_GENERIC_TYPE == SAMR)
#   include "pal_timer_hw.h"
#elif (PAL_GENERIC_TYPE == IMX6SX)

#endif
#ifdef ENABLE_HIGH_PRIO_TMR
#   include "pal_timer_high_prio.h"
#endif

/* === Types ================================================================ */

/*
 * Type of timer instance (usually used for specifying the trx id of the
 * dedicated timer).
 * This means a specific timer with a given timer id can be handled in parallel
 * for various instances (i.e. trx id's).
 */
typedef uint8_t timer_instance_id_t;

/* Type definition for timer elements */
typedef struct timer_element_tag
{
    /* Current Timer Id */
    uint16_t timer_id;

    /*
     * Current timer instance id
     * This parameter is basically specifying the trx id of the given timer
     * with the specified timer id.
     * It is usually zero for single trx systems, but may contain other values
     * depending on the value of NUM_TRX within the TAL
     */
    timer_instance_id_t timer_instance_id;

    /* Timeout in microseconds */
    uint32_t abs_exp_timer;

    /* Callback function to be executed on expiry of the timer */
    FUNC_PTR(timer_cb);

    /* Parameter to be passed to the callback function of the expired timer */
    void *param_cb;

    /*
     * Next timer which was started or has expired:
     * Used for maintaining the running or expired the timer queues.
     * This field stores the timer id of the next timer in the
     * (running and expired) queue. It is used only for regular timers.
     */
    uint_fast8_t next_timer_in_queue;
} timer_element_t;

/*
 * Type definition for callbacks for timer functions
 * The parameter (uint8_t *) is a pointer to the timer_element_t of the expired
 * timer.
 */
typedef void (*timer_expiry_cb_t)(timer_element_t *cb_timer_element);

/* === Externals ============================================================ */

extern uint_fast8_t running_timer_queue_head;
extern uint8_t running_timers;
extern volatile uint16_t sys_time;
extern timer_element_t timer_array[];
extern volatile bool timer_trigger;

/* === Macros ================================================================ */

/*
 * First timer ID form PAL layer
 * Do NOT CHANGE THIS
 *
 * Note: This is a pro forma definition in case a PAL timer is actually
 * required.
 */
#define PAL_FIRST_TIMER_ID          (0)

/*
 * The largest timeout in microseconds
 */
#define MAX_TIMEOUT                     (0x7FFFFFFF)

#if (PAL_GENERIC_TYPE == AVR) || (PAL_GENERIC_TYPE == XMEGA) || \
    (PAL_GENERIC_TYPE == SAM4) || (PAL_GENERIC_TYPE == SAMR)
/* These MCU families use us timer. */
#   define MAX_TIMEOUT_FINAL        (MAX_TIMEOUT)
#   define MIN_TIMEOUT_FINAL        (MIN_TIMEOUT)
#elif (PAL_GENERIC_TYPE == MEGA_RF) 
/* The Mega-RF MCU familie uses symbol time timer (16us). */
#   define MAX_TIMEOUT_FINAL        (MAX_TIMEOUT / PAL_US_PER_SYMBOLS)
#   define MIN_TIMEOUT_FINAL        (MIN_TIMEOUT / PAL_US_PER_SYMBOLS)
#elif (PAL_GENERIC_TYPE == IMX6SX) 

#else
#   error("Current PAL_GENERIC_TYPE not handled")
#endif  /* PAL_GENERIC_TYPE */

/*
 * Shift mask to obtain the 16-bit system time out of a 32-bit timeout
 */
#define SYS_TIME_SHIFT_MASK     (16)

/*
 * Mask to obtain the 16-bit H/W time out of a 32-bit timeout
 */
#define HW_TIME_MASK            (0xFFFF)

/* === Prototypes =========================================================== */

#ifdef __cplusplus
extern "C" {
#endif

    bool compare_time(uint32_t t1, uint32_t t2);
    uint32_t gettime(void);
    void prog_comp_reg(void);
    void timer_init(void);
    void timer_init_non_generic(void);
    void timer_service(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /* PAL_TIMER_H */
/* EOF */
