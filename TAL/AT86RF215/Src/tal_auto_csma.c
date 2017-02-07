/**
 * @file tal_auto_csma.c
 *
 * @brief This file handles CSMA / CA before frame transmission within the TAL.
 *
 * $Id: tal_auto_csma.c 37813 2015-09-02 14:18:22Z uwalter $
 *
 * @author    Atmel Corporation: http://www.atmel.com
 * @author    Support email: avr@atmel.com
 */
/*
 * Copyright (c) 2012, Atmel Corporation All rights reserved.
 *
 * Licensed under Atmel's Limited License Agreement --> EULA.txt
 */

#include "tal_config.h"

/* === INCLUDES ============================================================ */

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "pal.h"
#include "return_val.h"
#include "tal.h"
#include "ieee_const.h"
#include "tal_internal.h"
#include <signal.h> 

/* === TYPES =============================================================== */

/* === MACROS ============================================================== */

/* === GLOBALS ============================================================= */

static uint8_t NB[NUM_TRX];
static uint8_t BE[NUM_TRX];

/* === PROTOTYPES ========================================================== */

static void start_backoff(trx_id_t trx_id);
//static void cca_start(timer_element_t *cb_timer_element);
static void cca_start(union sigval v);

#ifdef SUPPORT_MODE_SWITCH
static void trigger_cca_meaurement(trx_id_t trx_id);
#endif

/* === IMPLEMENTATION ====================================================== */


/**
 * @brief Starts software-controlled CSMA.
 *
 * @param trx_id Transceiver identifier
 */
void csma_start(trx_id_t trx_id)
{
    /* Initialize CSMA variables */
    NB[trx_id] = 0;
    BE[trx_id] = tal_pib[trx_id].MinBE;

    if (BE[trx_id] == 0)
    {
        /* Collision avoidance is disabled during first iteration */
#ifdef SUPPORT_MODE_SWITCH
        if (tal_pib[trx_id].ModeSwitchEnabled)
        {
            tx_ms_ppdu(trx_id);
        }
        else
#endif
        {
            transmit_frame(trx_id, NO_CCA);
        }
    }
    else
    {
        /* Start backoff timer to trigger CCA */
        start_backoff(trx_id);
    }
}


/**
 * @brief Starts the timer for the backoff period and enables receiver.
 *
 * @param trx_id Transceiver identifier
 */
static void start_backoff(trx_id_t trx_id)
{
    /* Start backoff timer to trigger CCA */
    uint8_t backoff_8;
    backoff_8  = (uint8_t)(rand() & (((uint16_t)1 << BE[trx_id]) - 1));
    if (backoff_8 > 0)
    {
        uint16_t backoff_16;
        uint32_t backoff_duration_us;
        backoff_16 = backoff_8 * aUnitBackoffPeriod;
        backoff_duration_us = (uint32_t)tal_pib[trx_id].SymbolDuration_us * (uint32_t)backoff_16;
#ifdef REDUCED_BACKOFF_DURATION
        backoff_duration_us = REDUCED_BACKOFF_DURATION;
#endif

        retval_t status =
            pal_timer_start(TAL_T,
                            trx_id,
                            backoff_duration_us,
                            TIMEOUT_RELATIVE,
                            (FUNC_PTR())cca_start,
                            NULL);
        if (status != MAC_SUCCESS)
        {
            tx_done_handling(trx_id, status);
            return;
        }
        else
        {
            /* Switch to TRXOFF during backoff */
            tx_state[trx_id] = TX_BACKOFF;

            if ((trx_default_state[trx_id] == RF_TRXOFF) ||
                (tal_pib[trx_id].NumRxFramesDuringBackoff < tal_pib[trx_id].MaxNumRxFramesDuringBackoff))
            {
                if (trx_state[trx_id] != RF_TXPREP)
                {
                    switch_to_txprep(trx_id);
                }
            }
            else // RF_RX
            {
                /* Stay in Rx */
            }
        }
    }
    else // no backoff required
    {
        /* Start CCA immediately - no backoff */
        /*
         * The trx id is required in the callback function,
         * so create a proper timer element.
         */
        //timer_element_t timer_element;
        //timer_element.timer_instance_id = trx_id;
        union sigval v;
		v.sival_int=trx_id;
        cca_start(v);
    }
}


/**
 * @brief Start CCA.
 *
 * @param parameter Pointer to timer element containing the trx_id
 */
//static void cca_start(timer_element_t *cb_timer_element)
static void cca_start(union sigval v)

{
    /* Immediately store trx id from callback. */
    trx_id_t trx_id = (trx_id_t)(v.sival_int);;
    ASSERT((trx_id >= 0) (trx_id_t)(v.sival_int)&& (trx_id < NUM_TRX));

    /* ACK transmission is understood as channel busy */
    if (ack_transmitting[trx_id])
    {
        csma_continue(trx_id);
        return;
    }

    /* Check if trx is currently detecting a frame ota */
    if (trx_state[trx_id] == RF_RX)
    {
        CALC_REG_OFFSET(trx_id);
        uint8_t agc_freeze = pal_dev_bit_read(RF215_TRX, GET_REG_ADDR(SR_RF09_AGCC_FRZS));
        if (agc_freeze)
        {
            csma_continue(trx_id);
        }
        else
        {
#ifdef SUPPORT_MODE_SWITCH
            if (tal_pib[trx_id].ModeSwitchEnabled)
            {
                trigger_cca_meaurement(trx_id);
            }
            else
#endif
            {
                transmit_frame(trx_id, WITH_CCA);
            }
        }
    }
    else
    {
#ifdef SUPPORT_MODE_SWITCH
        if (tal_pib[trx_id].ModeSwitchEnabled)
        {
            trigger_cca_meaurement(trx_id);
        }
        else
#endif
        {
            transmit_frame(trx_id, WITH_CCA);
        }
    }
}


#ifdef SUPPORT_MODE_SWITCH
/**
 * @brief Triggers CCA measurement at transceiver
 *
 * @param trx_id Transceiver identifier
 */
static void trigger_cca_meaurement(trx_id_t trx_id)
{
    /* Trigger CCA measurement */
    CALC_REG_OFFSET(trx_id);

    /* Cancel any ongoing reception and ensure that TXPREP is reached. */
    if (trx_state[trx_id] == RF_TRXOFF)
    {
        switch_to_txprep(trx_id);
    }

    /* Disable BB */
    pal_dev_bit_write(RF215_TRX, GET_REG_ADDR(SR_BBC0_PC_BBEN), 0);

    /* Enable IRQ EDC */
    pal_dev_bit_write(RF215_TRX, GET_REG_ADDR(SR_RF09_IRQM_EDC), 1);

    /* CCA duration is already set by default; see apply_phy_settings() */
    /* Setup and start energy detection */
    pal_dev_bit_write(RF215_TRX, GET_REG_ADDR(SR_RF09_AGCC_FRZC), 0); // Ensure AGC is not hold
    if (trx_state[trx_id] != RF_RX)
    {
        stop_rpc(trx_id);
        pal_dev_reg_write(RF215_TRX, GET_REG_ADDR(RG_RF09_CMD), RF_RX);
        pal_timer_delay(tal_pib[trx_id].agc_settle_dur); // allow filters to settle
        trx_state[trx_id] = RF_RX;
    }
    tx_state[trx_id] = TX_CCA;
    /* Start single ED measurement; use reg_write - it's the only subregister */
    pal_dev_reg_write(RF215_TRX, GET_REG_ADDR(RG_RF09_EDC), RF_EDSINGLE);

    /* Wait for EDC IRQ and handle it within cca_done_handling() */
}
#endif


#ifdef SUPPORT_MODE_SWITCH
/**
 * @brief Callback function for CCA completion.
 *
 * @param trx_id Transceiver identifier
 */
void cca_done_handling(trx_id_t trx_id)
{
    switch_to_txprep(trx_id); /* Leave state Rx */

    /* Switch BB on again */
    CALC_REG_OFFSET(trx_id);
    pal_dev_bit_write(RF215_TRX, GET_REG_ADDR(SR_BBC0_PC_BBEN), 1);

    /* Determine if channel is idle */
    if (tal_current_ed_val[trx_id] < tal_pib[trx_id].CCAThreshold)
    {
        /* Idle */
        tx_ms_ppdu(trx_id);
    }
    else
    {
        /* Busy */
        csma_continue(trx_id);
    }
}
#endif


/**
 * @brief Continues CSMA; handles next CSMA retry.
 *
 * @param trx_id Transceiver identifier
 */
void csma_continue(trx_id_t trx_id)
{
    NB[trx_id]++;
    
    if (NB[trx_id] > tal_pib[trx_id].MaxCSMABackoffs)
    {
        tx_done_handling(trx_id, MAC_CHANNEL_ACCESS_FAILURE);
    }
    else
    {
        BE[trx_id]++;
        if (BE[trx_id] > tal_pib[trx_id].MaxBE)
        {
            BE[trx_id] = tal_pib[trx_id].MaxBE;
        }
        /* Start backoff timer to trigger CCA */
        start_backoff(trx_id);
    }
}

/* EOF */
