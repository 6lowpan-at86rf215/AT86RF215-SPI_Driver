/**
 * @file tal_rx_enable.c
 *
 * @brief File provides functionality supporting RX-Enable feature.
 *
 * $Id: tal_rx_enable.c 37587 2015-06-23 09:38:53Z uwalter $
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
#include <string.h>
#include <stdbool.h>
#include "pal.h"
#include "return_val.h"
#include "tal.h"
#include "ieee_const.h"
#include "tal_config.h"
#include "bmm.h"
#include "qmm.h"
#include "tal_internal.h"

/* === TYPES =============================================================== */

/* === MACROS ============================================================== */

/* === GLOBALS ============================================================= */

/* === PROTOTYPES ========================================================== */

/* === IMPLEMENTATION ====================================================== */

/**
 * @brief Switches receiver on or off
 *
 * This function switches the receiver on (PHY_RX_ON) or off (PHY_TRX_OFF).
 *
 * @param trx_id Transceiver identifier
 * @param state New state of receiver
 *
 * @return
 *      - @ref TAL_BUSY if the TAL state machine cannot switch receiver on or off,
 *      - @ref PHY_TRX_OFF if receiver has been switched off, or
 *      - @ref PHY_RX_ON otherwise.
 *
 * @ingroup apiTalApi
 */
uint8_t tal_rx_enable(trx_id_t trx_id, uint8_t state)
{
	ASSERT((trx_id >= 0) && (trx_id < NUM_TRX));
	
    uint8_t ret_val;

    if (tal_state[trx_id] == TAL_SLEEP)
    {
        return TAL_TRX_ASLEEP;
    }

    /*
     * Trx can only be enabled if TAL is not busy;
     * i.e. if TAL is IDLE.
     */
    if (tal_state[trx_id] != TAL_IDLE)
    {
        return TAL_BUSY;
    }

    /*
     * Check current state
     */
    if ((state == PHY_TRX_OFF) && (trx_state[trx_id] == RF_TRXOFF))
    {
        return PHY_TRX_OFF;
    }
    if ((state == PHY_RX_ON) && (trx_state[trx_id] == RF_RX))
    {
        return PHY_RX_ON;
    }

    if (state == PHY_TRX_OFF)
    {
        /*
         * If the rx needs to be switched off,
         * we are not interested in a frame that is currently being received.
         */
        CALC_REG_OFFSET(trx_id);
        pal_dev_reg_write(RF215_TRX, GET_REG_ADDR(RG_RF09_CMD), RF_TRXOFF);
#if (defined SUPPORT_FSK) || (defined SUPPORT_OQPSK)
        stop_rpc(trx_id);
#endif
        trx_state[trx_id] = RF_TRXOFF;
        tal_buf_shortage[trx_id] = false;
        ret_val = PHY_TRX_OFF;
#ifdef ENABLE_FTN_PLL_CALIBRATION
        stop_ftn_timer(trx_id);
#endif  /* ENABLE_FTN_PLL_CALIBRATION */
        trx_default_state[trx_id] = RF_TRXOFF;
    }
    else
    {
        switch_to_txprep(trx_id);
        switch_to_rx(trx_id);
#if (defined SUPPORT_FSK) || (defined SUPPORT_OQPSK)
        start_rpc(trx_id);
#endif
        ret_val = PHY_RX_ON;
#ifdef ENABLE_FTN_PLL_CALIBRATION
        /*
         * Start the FTN timer.
         *
         * Since we do not have control when tal_rx_enable is
         * from the NHLE, and this timer is running for quite some
         * time, we first need to check, if this timer has not been started
         * been already. In this case we leave it running.
         * Otherwise this would lead to assertions.
         */
        if (!pal_is_timer_running(TAL_T_CALIBRATION,
                                  trx_id))
        {
            start_ftn_timer(trx_id);
        }
#endif  /* ENABLE_FTN_PLL_CALIBRATION */
        trx_default_state[trx_id] = RF_RX;
    }

    return ret_val;
}

/* EOF */
