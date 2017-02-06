/**
 * @file tal_init.c
 *
 * @brief This file implements functions for initializing TAL and reset.
 *
 * $Id: tal_init.c 37810 2015-08-28 14:00:55Z uwalter $
 *
 * @author    Atmel Corporation: http://www.atmel.com
 * @author    Support email: avr@atmel.com
 */
/*
 * Copyright (c) 2012, Atmel Corporation All rights reserved.
 *
 * Licensed under Atmel's Limited License Agreement --> EULA.txt
 */

/* === INCLUDES ============================================================ */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "pal.h"
#include "return_val.h"
#include "tal.h"
#include "ieee_const.h"
#include "tal_pib.h"
#include "stack_config.h"
#include "bmm.h"
#include "qmm.h"
#include "pal.h"
#include "tal.h"
#include "tal_internal.h"
#include "tal_config.h"
#ifdef SUPPORT_TFA
#include "tfa.h"
#endif
#include "mac_build_config.h"

/* === MACROS ============================================================== */

/* === GLOBALS ============================================================= */

/* === PROTOTYPES ========================================================== */

static retval_t trx_reset(trx_id_t trx_id);
static void cleanup_tal(trx_id_t trx_id);
static void trx_init(void);

/* === IMPLEMENTATION ====================================================== */


/**
 * @brief Initializes the TAL
 *
 * This function is called to initialize the TAL. The transceiver is
 * initialized, the TAL PIBs are set to their default values, and the TAL state
 * machine is set to TAL_IDLE state.
 *
 * @return MAC_SUCCESS  if the transceiver state is changed to TRX_OFF and the
 *                 current device part number and version number are correct;
 *         FAILURE otherwise
 */
retval_t tal_init(void)
{
    /* Init the PAL and by this means also the transceiver interface */
    if (pal_init() != MAC_SUCCESS)
    {
        return FAILURE;
    }

    /* Reset trx */
    if (trx_reset(RFBOTH) != MAC_SUCCESS)
    {
printf("trx_reset fail\n");

        return FAILURE;
    }
printf("Check if RF215 is connected\n");
    /* Check if RF215 is connected */
#if (defined RF215v1) || (defined RF215v2) || (defined RF215v3) || (defined RF215Mv1)
    if ((pal_dev_reg_read(RF215_TRX, RG_RF_PN) != 0x34) ||
#elif (defined RF215Mv2)
    if ((pal_dev_reg_read(RF215_TRX, RG_RF_PN) != 0x36) ||
#else
#   error "No part number defined"
#endif
#if (defined RF215v1) || (defined RF215Mv1)
        (pal_dev_reg_read(RF215_TRX, RG_RF_VN) != 0x01))
#elif (defined RF215v2) || (defined RF215Mv2)
        (pal_dev_reg_read(RF215_TRX, RG_RF_VN) != 0x02))
#elif (defined RF215v3)
        (pal_dev_reg_read(RF215_TRX, RG_RF_VN) != 0x03))
#else
#   error "No IC version defined"
#endif
    {
        return FAILURE;
    }

    /* Initialize trx */
    trx_init();

    /* Initialize the buffer management */
    bmm_buffer_init();

    /* Configure both trx and set default PIB values */
    for (trx_id_t trx_id = (trx_id_t)0; trx_id < NUM_TRX; trx_id++)
    {
        /* Configure transceiver */
        trx_config(trx_id);
#ifdef RF215v1
        /* Calibrate LO */
        calibrate_LO(trx_id);
#endif

        /* Set the default PIB values */
        init_tal_pib(trx_id); /* see 'tal_pib.c' */
        calculate_pib_values(trx_id);

        /*
         * Write all PIB values to the transceiver
         * that are needed by the transceiver itself.
         */
        write_all_tal_pib_to_trx(trx_id); /* see 'tal_pib.c' */
        config_phy(trx_id);

        tal_rx_buffer[trx_id] = bmm_buffer_alloc(LARGE_BUFFER_SIZE);
        if (tal_rx_buffer[trx_id] == NULL)
        {
            return FAILURE;
        }
        else
        {
            /* Fill trx_id in new buffer */
            frame_info_t *frm_info = (frame_info_t *)BMM_BUFFER_POINTER(tal_rx_buffer[trx_id]);
            frm_info->trx_id = trx_id;
        }

        /* Init incoming frame queue */
        qmm_queue_init(&tal_incoming_frame_queue[trx_id]);

        tal_state[trx_id] = TAL_IDLE;
        tx_state[trx_id] = TX_IDLE;
    }

    /* Init seed of rand() */
    tal_generate_rand_seed();

    /*
     * Configure interrupt handling.
     * Install a handler for the radio and the baseband interrupt.
     */
    pal_dev_irq_flag_clr(RF215_TRX);
    pal_dev_irq_init(RF215_TRX, trx_irq_handler_cb);
    pal_dev_irq_en(RF215_TRX);   /* Enable transceiver main interrupt. */

#if ((defined SUPPORT_FSK) && (defined SUPPORT_MODE_SWITCH))
    init_mode_switch();
#endif

    return MAC_SUCCESS;
} /* tal_init() */


/**
 * @brief Configures the transceiver
 *
 * This function is called to configure a certain transceiver (RF09 or RF24)
 * after trx sleep or reset or power on.
 *
 * @param trx_id Transceiver identifier
 */
void trx_config(trx_id_t trx_id)
{
    CALC_REG_OFFSET(trx_id);

#ifdef ENABLE_TSTAMP
    pal_dev_reg_write(RF215_TRX, GET_REG_ADDR(RG_BBC0_IRQM),
                      BB_IRQ_RXFE | BB_IRQ_TXFE | BB_IRQ_RXFS);
#else
    pal_dev_reg_write(RF215_TRX, GET_REG_ADDR(RG_BBC0_IRQM),
                      BB_IRQ_RXFE | BB_IRQ_TXFE);
#endif
    /* Configure RF */
    pal_dev_reg_write(RF215_TRX, GET_REG_ADDR(RG_RF09_IRQM), RF_IRQ_BATLOW | RF_IRQ_WAKEUP);

    /* Enable frame filter */
    pal_dev_bit_write(RF215_TRX, GET_REG_ADDR(SR_BBC0_AFC0_AFEN0), 1);


#if (defined MEASURE_TIME_OF_FLIGHT)
    /* Enable automatic time of flight measurement */
    /* bit 3 CAPRXS, bit 2 RSTTXS, bit 0 EN */
    uint8_t cnt_cfg = CNTC_EN_MASK | CNTC_RSTTXS_MASK | CNTC_CAPRXS_MASK;
    pal_dev_reg_write(RF215_TRX, GET_REG_ADDR(RG_BBC0_CNTC), cnt_cfg);
#endif /* #if (defined MEASURE_TIME_OF_FLIGHT) */

#ifndef USE_TXPREP_DURING_BACKOFF
    /* Keep analog voltage regulator on during TRXOFF */
    pal_dev_bit_write(RF215_TRX, GET_REG_ADDR(SR_RF09_AUXS_AVEN), 1);
#endif

    /* Enable AACK */
    pal_dev_reg_write(RF215_TRX, GET_REG_ADDR(RG_BBC0_AMCS), AMCS_AACK_MASK);
    /* Set data pending for ACK frames to 1 for all address units */
    pal_dev_reg_write(RF215_TRX, GET_REG_ADDR(RG_BBC0_AMAACKPD), 0x0F);

#ifdef SUPPORT_MODE_SWITCH
    /* Use raw mode for mode switch PPDU in the not-inverted manner */
    pal_dev_bit_write(RF215_TRX, GET_REG_ADDR(SR_BBC0_FSKC4_RAWRBIT), 0);
#endif
}


/**
 * @brief Initializes the transceiver
 *
 * This function is called to initialize the general transceiver functionality
 * after power or IC reset.
 */
static void trx_init(void)
{
    /*
     * Configure generic trx functionality
     * Configure Trx registers that are reset during DEEP_SLEEP and IC reset
     * I.e.: RF_CFG, RF_BMDVC, RF_XOC, RF_IQIFC0, RF_IQIFC1, RF_IQIFC2
     */
#ifdef TRX_IRQ_POLARITY
#if (TRX_IRQ_POLARITY == 1)
    pal_dev_bit_write(RF215_TRX, SR_RF_CFG_IRQP, 1); /* 1 = active low */
#endif
#endif
#if (TRX_CLOCK_OUTPUT_SELECTION != 1)
    pal_dev_bit_write(RF215_TRX, SR_RF_CLKO_OS, TRX_CLOCK_OUTPUT_SELECTION);
#endif
}


/**
 * @brief Resets TAL state machine and sets the default PIB values if requested
 *
 * @param trx_id Transceiver identifier
 * @param set_default_pib Defines whether PIB values need to be set
 *                        to its default values
 *
 * @return
 *      - @ref MAC_SUCCESS if the transceiver state is changed to TRX_OFF
 *      - @ref FAILURE otherwise
 * @ingroup apiTalApi
 */
retval_t tal_reset(trx_id_t trx_id, bool set_default_pib)
{
    rf_cmd_state_t previous_trx_state[NUM_TRX];

    /* Clean TAL and removed any pending tasks */
    if (trx_id == RFBOTH)
    {
        for (trx_id_t i = (trx_id_t)0; i < NUM_TRX; i++)
        {
            /* Clean TAL and removed any pending tasks */
            cleanup_tal(i);
        }
    }
    else
    {
        cleanup_tal(trx_id);
    }

    previous_trx_state[RF09] = trx_state[RF09];
    previous_trx_state[RF24] = trx_state[RF24];

    /* Reset the actual device or part of the device */
    if (trx_reset(trx_id) != MAC_SUCCESS)
    {
        return FAILURE;
    }

    /* Init Trx if necessary, e.g. trx was in deep sleep */
    if (((previous_trx_state[RF09] == RF_SLEEP) &&
         (previous_trx_state[RF24] == RF_SLEEP)) || (trx_id == RFBOTH))
    {
        trx_init(); /* Initialize generic trx functionality */
    }

    if (trx_id == RFBOTH)
    {
        for (trx_id_t i = (trx_id_t)0; i < NUM_TRX; i++)
        {
            /* Configure the transceiver register values. */
            trx_config(i);

            if (set_default_pib)
            {
                /* Set the default PIB values */
                init_tal_pib(i); /* see 'tal_pib.c' */
                calculate_pib_values(i);
            }
            else
            {
                /* nothing to do - the current TAL PIB attribute values are used */
            }
            write_all_tal_pib_to_trx(i); /* see 'tal_pib.c' */
            config_phy(i);

            /* Reset TAL variables. */
            tal_state[i] = TAL_IDLE;
            tx_state[i] = TX_IDLE;
        }
    }
    else
    {
        /* Configure the transceiver register values. */
        trx_config(trx_id);

        if (set_default_pib)
        {
            /* Set the default PIB values */
            init_tal_pib(trx_id); /* see 'tal_pib.c' */
            calculate_pib_values(trx_id);
        }
        else
        {
            /* nothing to do - the current TAL PIB attribute values are used */
        }
        write_all_tal_pib_to_trx(trx_id); /* see 'tal_pib.c' */
        config_phy(trx_id);

        /* Reset TAL variables. */
        tal_state[trx_id] = TAL_IDLE;
        tx_state[trx_id] = TX_IDLE;
    }

    /*
     * Configure interrupt handling.
     * Install a handler for the transceiver interrupt.
     */
    pal_dev_irq_init(RF215_TRX, trx_irq_handler_cb);
    pal_dev_irq_en(RF215_TRX);   /* Enable transceiver main interrupt. */

    return MAC_SUCCESS;
}


/**
 * @brief Resets transceiver(s)
 *
 * @param trx_id Transceiver identifier
 *
 * @return MAC_SUCCESS  if the transceiver returns TRX_OFF
 *         FAILURE otherwise
 */
static retval_t trx_reset(trx_id_t trx_id)
{
    retval_t status = MAC_SUCCESS;

    ENTER_TRX_REGION();
printf("trx_reset run\n");

    uint32_t start_time;
    uint32_t current_time;
    pal_get_current_time(&start_time);

    /* Trigger reset */
    if (trx_id == RFBOTH)
    {
        TAL_RF_IRQ_CLR_ALL(RF09);
        TAL_RF_IRQ_CLR_ALL(RF24);

        /* Apply reset pulse; low active */
        PAL_DEV_RST_LOW(RF215_TRX);
        PAL_WAIT_1_US();
        PAL_DEV_RST_HIGH(RF215_TRX);
    }
    else // only a single trx_id; i.e. RF09 or RF24
    {
        TAL_RF_IRQ_CLR_ALL(trx_id);

        /* Trigger reset of device */
        CALC_REG_OFFSET(trx_id);
    
        pal_dev_reg_write(RF215_TRX, GET_REG_ADDR(RG_RF09_CMD), RF_RESET);
    }

    /* Wait for IRQ line */
    while (1)
    {
        /*
         * @ToDo: Use a different macro for IRQ line; the polarity might be
         * different after reset
         */
        if (PAL_DEV_IRQ_GET(RF215_TRX) == high)
        {
        	printf("GPIO-IRQ PIN is high, AT86rf215 rest complete\n");
            break;
        }
        /* Handle timeout */
        pal_get_current_time(&current_time);
        // @ToDo: Remove magic number
        if (pal_sub_time_us(current_time, start_time) > 1000)
        {
            status = FAILURE;
            break;
        }
    }

    /* Check for trx status */
    if (status == MAC_SUCCESS) // IRQ has been fired and no timeout has occurred
    {
        if (trx_id == RFBOTH)
        {
            for (uint8_t i = (trx_id_t)0; i < NUM_TRX; i++)
            {
                CALC_REG_OFFSET(i);
                trx_state[i] = (rf_cmd_state_t)pal_dev_reg_read(RF215_RF, GET_REG_ADDR(RG_RF09_STATE));
                if (trx_state[i] != RF_TRXOFF)
                {
printf("RF_TRXOFF error\n");

                    status = FAILURE;
                }
            }
        }
        else // single trx
        {
            CALC_REG_OFFSET(trx_id);
            trx_state[trx_id] = (rf_cmd_state_t)pal_dev_reg_read(RF215_TRX, GET_REG_ADDR(RG_RF09_STATE));
            if (trx_state[trx_id] != RF_TRXOFF)
            {
                status = FAILURE;
            }
        }
    }

    /* Get all IRQ status information */
    trx_irq_handler_cb();
    if (trx_id == RFBOTH)
    {
        for (uint8_t i = (trx_id_t)0; i < NUM_TRX; i++)
        {
            TAL_RF_IRQ_CLR(i, RF_IRQ_WAKEUP);
        }
    }
    else // single trx
    {
        TAL_RF_IRQ_CLR(trx_id, RF_IRQ_WAKEUP);
    }

    pal_dev_irq_flag_clr(RF215_TRX);

    LEAVE_TRX_REGION();

    return status;
}


/**
 * @brief Cleanup TAL
 *
 * @param trx_id Transceiver identifier
 */
static void cleanup_tal(trx_id_t trx_id)
{
    /* Clear all running TAL timers (for this trx id). */
    ENTER_CRITICAL_REGION();

    /* Traverse through all timer ids. */
    for (tal_timer_id_t timer_id = (tal_timer_id_t)TAL_FIRST_TIMER_ID;
         timer_id <= TAL_LAST_TIMER_ID;
         timer_id++)
    {
        pal_timer_stop(timer_id, trx_id);
    }

    LEAVE_CRITICAL_REGION();

    /* Clear TAL Incoming Frame queue and free used buffers. */
    while (tal_incoming_frame_queue[trx_id].size > 0)
    {
        buffer_t *frame = qmm_queue_remove(&tal_incoming_frame_queue[trx_id], NULL);
        if (NULL != frame)
        {
            bmm_buffer_free(frame);
        }
    }
    /* Get new TAL Rx buffer if necessary */
    if (tal_rx_buffer[trx_id] == NULL)
    {
        tal_rx_buffer[trx_id] = bmm_buffer_alloc(LARGE_BUFFER_SIZE);
    }
    /* Handle buffer shortage */
    if (tal_rx_buffer[trx_id] == NULL)
    {
        tal_buf_shortage[trx_id] = true;
    }
    else
    {
        tal_buf_shortage[trx_id] = false;
        /* Fill trx_id in new buffer */
        frame_info_t *frm_info = (frame_info_t *)BMM_BUFFER_POINTER(tal_rx_buffer[trx_id]);
        frm_info->trx_id = trx_id;
    }
}


/* EOF */
