/**
 * @file tal_auto_ack.c
 *
 * @brief This file implements acknowledgement handling.
 *
 * $Id: tal_auto_ack.c 37813 2015-09-02 14:18:22Z uwalter $
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
#include <inttypes.h>
#include "pal.h"
#include "return_val.h"
#include "tal.h"
#include "ieee_const.h"
#include "stack_config.h"
#include "tal_pib.h"
#include "tal_internal.h"
#include <signal.h> 

/* === TYPES =============================================================== */

/* === MACROS ============================================================== */

/* === GLOBALS ============================================================= */

/* === PROTOTYPES ========================================================== */

//static void ack_timout_cb(timer_element_t *cb_timer_element);
static void ack_timout_cb(union sigval v);

/* === IMPLEMENTATION ====================================================== */



/* --- ACK transmission ---------------------------------------------------- */


/**
 * @brief Handles end of ACK transmission
 *
 * This function is called with the TXFE IRQ.
 * It handles further processing after an ACK has been transmitted.
 *
 * @param trx_id Id of the corresponding trx
 */
void ack_transmission_done(trx_id_t trx_id)
{
    ack_transmitting[trx_id] = false;

#ifdef SUPPORT_FSK
    if (tal_pib[trx_id].RPCEnabled && tal_pib[trx_id].phy.modulation == FSK)
    {
        /* Configure preamble length for reception */
        CALC_REG_OFFSET(trx_id);
        pal_dev_reg_write(RF215_TRX, GET_REG_ADDR(RG_BBC0_FSKPLL),
                          (uint8_t)(tal_pib[trx_id].FSKPreambleLengthMin & 0xFF));
        pal_dev_bit_write(RF215_TRX, GET_REG_ADDR(SR_BBC0_FSKC1_FSKPLH),
                          (uint8_t)(tal_pib[trx_id].FSKPreambleLengthMin >> 8));
    }
#endif

#ifdef MEASURE_ON_AIR_DURATION
    tal_pib[trx_id].OnAirDuration += tal_pib[trx_id].ACKDuration_us;
#endif

    complete_rx_transaction(trx_id);

    switch (tx_state[trx_id])
    {
        case TX_IDLE:
            switch_to_rx(trx_id);
            break;

         case TX_DEFER:
            /* Continue with deferred transmission */
            continue_deferred_transmission(trx_id);
            break;

         default:
            ASSERT("Unexpected tx_state" == 0);
            break;
    }
}


/* --- ACK reception ------------------------------------------------------- */


/**
 * @brief Checks if received frame is an ACK frame
 *
 * @param trx_id Transceiver identifier
 *
 * @return true if frame is an ACK frame, else false
 */
bool is_frame_an_ack(trx_id_t trx_id)
{
    bool ret;

    /* Check frame length */
    if (rx_frm_info[trx_id]->len_no_crc == 3)
    {
        /* Check frame type and frame version */
        if ((rx_frm_info[trx_id]->mpdu[0] & FCF_FRAMETYPE_ACK) &&
            (((rx_frm_info[trx_id]->mpdu[1] >> FCF1_FV_SHIFT) & 0x03) <= FCF_FRAME_VERSION_2006))
        {
            ret = true;
        }
        else
        {
            ret = false;
        }
    }
    else
    {
        ret = false;
    }

    return ret;
}


/**
 * @brief Checks if received ACK is an valid ACK frame
 *
 * @param trx_id Transceiver identifier
 *
 * @return true if ACK frame is valid, else false
 */
bool is_ack_valid(trx_id_t trx_id)
{
    bool ret;

    /* Check sequence number */
    if (rx_frm_info[trx_id]->mpdu[PL_POS_SEQ_NUM] == mac_frame_ptr[trx_id]->mpdu[PL_POS_SEQ_NUM])
    {
        ret = true;
    }
    else
    {
        ret = false;
    }

    return ret;
}


/**
 * @brief Starts the timer to wait for an ACK reception
 *
 * @param trx_id Id of the corresponding trx
 */
void start_ack_wait_timer(trx_id_t trx_id)
{
    tx_state[trx_id] = TX_WAITING_FOR_ACK;

    retval_t status =
        pal_timer_start(TAL_T,
                        trx_id,
                        tal_pib[trx_id].ACKWaitDuration,
                        TIMEOUT_RELATIVE,
                        (FUNC_PTR())ack_timout_cb,
                        NULL);
	ASSERT(status == MAC_SUCCESS);
    if (status != MAC_SUCCESS)
    {
        CALC_REG_OFFSET(trx_id);
        pal_dev_reg_write(RF215_TRX, GET_REG_ADDR(RG_RF09_CMD), RF_TRXOFF);
        trx_state[trx_id] = RF_TRXOFF;
        tx_done_handling(trx_id, status);
    }
    else
    {
        /* Configure frame filter to receive only ACK frames */
        CALC_REG_OFFSET(trx_id);
        pal_dev_reg_write(RF215_TRX, GET_REG_ADDR(RG_BBC0_AFFTM), ACK_FRAME_TYPE_ONLY);
        /* Sync with Trx state; due to Tx2Rx the transceiver is alreadyswitches automatically to Rx */
        trx_state[trx_id] = RF_RX;
    }
}


/**
 * @brief Callback function for ACK timeout
 *
 * This function is called if the ACK timeout timer fires.
 *
 * @param parameter Pointer to trx_id
 */
//void ack_timout_cb(timer_element_t *cb_timer_element)
void ack_timout_cb(union sigval v)
{
    /* Immediately store trx id from callback. */
    trx_id_t trx_id = (trx_id_t)(v.sival_int);
    ASSERT((trx_id >= 0) && (trx_id < NUM_TRX));

    switch_to_txprep(trx_id);

    /* Configure frame filter to receive all allowed frame types */
    /* Re-store frame filter to pass "normal" frames */
    CALC_REG_OFFSET(trx_id);
#ifdef SUPPORT_FRAME_FILTER_CONFIGURATION
    pal_dev_reg_write(RF215_TRX, GET_REG_ADDR(RG_BBC0_AFFTM), tal_pib[trx_id].frame_types);
#else
    pal_dev_reg_write(RF215_TRX, GET_REG_ADDR(RG_BBC0_AFFTM), DEFAULT_FRAME_TYPES);
#endif

    tx_done_handling(trx_id, MAC_NO_ACK);
}

/*  EOF */
