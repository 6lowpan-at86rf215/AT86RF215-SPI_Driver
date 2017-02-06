/**
 * @file tal_ed.c
 *
 * @brief This file implements ED Scan
 *
 * $Id: tal_ed.c 37139 2015-04-01 12:47:11Z sschneid $
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

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "pal.h"
#include "return_val.h"
#include "tal.h"
#include "ieee_const.h"
#include "tal_config.h"
#include "tal_internal.h"
#include "mac_build_config.h"

/* === TYPES =============================================================== */

/* === MACROS ============================================================== */

/**
 * Values used for ED scaling in dBm
 */
#define UPPER_ED_LIMIT      -30
#define LOWER_ED_LIMIT      -100

/* === GLOBALS ============================================================= */

/**
 * The peak_ed_level is the maximum ED value received from the transceiver for
 * the specified Scan Duration.
 */
#if (MAC_SCAN_ED_REQUEST_CONFIRM == 1)
static int8_t max_ed_level[NUM_TRX];
static uint32_t sampler_counter[NUM_TRX];
#endif

/* === PROTOTYPES ========================================================== */

/* === IMPLEMENTATION ====================================================== */


/**
 * @brief Starts ED Scan
 *
 * This function starts an ED Scan for the scan duration specified by the
 * MAC layer. The result is returned from the TAL by calling tal_ed_end_cb().
 *
 * @param trx_id Transceiver identifier
 * @param scan_duration Specifies the ED scan duration in superframe duration
 *
 * @return MAC_SUCCESS - ED scan duration timer started successfully
 *         TAL_BUSY - TAL is busy servicing the previous request from MAC
 *         TAL_TRX_ASLEEP - Transceiver is currently sleeping
 *         FAILURE otherwise
 */
#if (MAC_SCAN_ED_REQUEST_CONFIRM == 1) || (defined DOXYGEN)
retval_t tal_ed_start(trx_id_t trx_id, uint8_t scan_duration)
{
    ASSERT((trx_id >= 0) && (trx_id < NUM_TRX));

    uint16_t sample_duration;
    CALC_REG_OFFSET(trx_id);

    /*
     * Check if the TAL is in idle state. Only in idle state it can
     * accept and ED request from the MAC.
     */
    if (tal_state[trx_id] == TAL_SLEEP)
    {
        return TAL_TRX_ASLEEP;
    }

    if (TAL_IDLE != tal_state[trx_id])
    {
        ASSERT("TAL is TAL_BUSY" == 0);
        return TAL_BUSY;
    }

    max_ed_level[trx_id] = -127;   // set to min value

    /* Store TRX state before entering Tx transaction */
    if ((trx_state[trx_id] == RF_RX) || (trx_state[trx_id] == RF_TXPREP))
    {
        trx_default_state[trx_id] = RF_RX;
    }

    switch_to_txprep(trx_id);

    /* Disable BB */
    pal_dev_bit_write(RF215_TRX, GET_REG_ADDR(SR_BBC0_PC_BBEN), 0);

    /* Setup and start energy detection, ensure AGC is not hold */
    pal_dev_bit_write(RF215_TRX, GET_REG_ADDR(SR_RF09_AGCC_FRZC), 0);

    /* Setup energy measurement averaging duration */
    sample_duration = ED_SAMPLE_DURATION_SYM * tal_pib[trx_id].SymbolDuration_us;
    
    set_ed_sample_duration(trx_id, sample_duration);

    /* Enable EDC IRQ */
    pal_dev_bit_write(RF215_TRX, GET_REG_ADDR(SR_RF09_IRQM_EDC), 1);

    /* Calculate the number of samples */
    sampler_counter[trx_id] = aBaseSuperframeDuration
                              * ((1UL << scan_duration) + 1);
    sampler_counter[trx_id] = sampler_counter[trx_id] / ED_SAMPLE_DURATION_SYM;
    
    /* Set RF to Rx */
    pal_dev_reg_write(RF215_TRX, GET_REG_ADDR(RG_RF09_CMD), RF_RX);
    trx_state[trx_id] = RF_RX;
    pal_timer_delay(tal_pib[trx_id].agc_settle_dur); // allow filters to settle

    tal_state[trx_id] = TAL_ED_SCAN;

    /* Start energy measurement */
    pal_dev_bit_write(RF215_TRX, GET_REG_ADDR(SR_RF09_EDC_EDM), RF_EDCONT);

    return MAC_SUCCESS;
}
#endif /* #if (MAC_SCAN_ED_REQUEST_CONFIRM == 1) */


/**
 * @brief Handles ED scan end interrupt
 *
 * This function handles an ED done interrupt from the transceiver.
 *
 * @param trx_id Transceiver identifier
 */
void handle_ed_end_irq(trx_id_t trx_id)
{
    /* Capture ED value for current frame / ED scan */
    CALC_REG_OFFSET(trx_id);
    tal_current_ed_val[trx_id] = pal_dev_reg_read(RF215_TRX, GET_REG_ADDR(RG_RF09_EDV));
  
#if (defined SUPPORT_MODE_SWITCH) 
    if (tx_state[trx_id] == TX_CCA)
    {
        cca_done_handling(trx_id);
        return;
    }
#endif

#if (MAC_SCAN_ED_REQUEST_CONFIRM == 1)
    if (tal_state[trx_id] == TAL_ED_SCAN)
    {
        /*
         * Update the peak ED value received, if greater than the previously
         * read ED value.
         */
        if (tal_current_ed_val[trx_id] > max_ed_level[trx_id])
        {
            max_ed_level[trx_id] = tal_current_ed_val[trx_id];
        }

        sampler_counter[trx_id]--;
       
        if (sampler_counter[trx_id] == 0)
        {
            /* Keep RF in Rx state */
            /* Stop continuous energy detection */
            pal_dev_bit_write(RF215_TRX, GET_REG_ADDR(SR_RF09_EDC_EDM), RF_EDAUTO);
            /* Restore ED average duration for CCA */
            set_ed_sample_duration(trx_id, tal_pib[trx_id].CCADuration_us);
            /* Switch BB on again */
            pal_dev_bit_write(RF215_TRX, GET_REG_ADDR(SR_BBC0_PC_BBEN), 1);
            tal_state[trx_id] = TAL_IDLE;
            /* Set trx state for leaving ED scan */
            if (trx_default_state[trx_id] == RF_RX)
            {
                switch_to_rx(trx_id);
            }
            else
            {
                pal_dev_reg_write(RF215_TRX, GET_REG_ADDR(RG_RF09_CMD), RF_TRXOFF);
            }
            /* Scale result to 0xFF */
            uint8_t ed = scale_ed_value(max_ed_level[trx_id]);

            /* Disable EDC IRQ again */
            pal_dev_bit_write(RF215_TRX, GET_REG_ADDR(SR_RF09_IRQM_EDC), 0);

            tal_ed_end_cb(trx_id, ed);
        }
        else
        {
            /* Wait for the next interrupt */
        }
    }
#endif /* #if (MAC_SCAN_ED_REQUEST_CONFIRM == 1) */
}


/**
 * @brief Sets the energy measurement duration
 *
 * @param trx_id Transceiver identifier
 * @param sample_duration_us Sample duration in us
 */
void set_ed_sample_duration(trx_id_t trx_id, uint16_t sample_duration_us)
{
    uint8_t dtb;
    uint8_t df;

    if ((sample_duration_us % 128) == 0)
    {
        dtb = 3;
        df = sample_duration_us / 128;
    }
    else if ((sample_duration_us % 32) == 0)
    {
        dtb = 2;
        df = sample_duration_us / 32;
    }
    else if ((sample_duration_us % 8) == 0)
    {
        dtb = 1;
        df = sample_duration_us / 8;
    }
    else
    {
        dtb = 0;
        df = sample_duration_us / 2;
    }

    CALC_REG_OFFSET(trx_id);
    pal_dev_reg_write(RF215_TRX, GET_REG_ADDR(RG_RF09_EDD), ((df << 2) | dtb));
}


/**
 * @brief Scale ED value
 *
 * This function scales the trx ED value to the range 0x00 - 0xFF.
 *
 * @param ed RF215 register value EDV.
 *
 * @return Scaled ED value
 */
uint8_t scale_ed_value(int8_t ed)
{
    uint8_t result;

    if (ed == 127)
    {
        result = 0x00;
    }
    else if (ed >= UPPER_ED_LIMIT)
    {
        result = 0xFF;
    }
    else if (ed <= LOWER_ED_LIMIT)
    {
        result = 0x00;
    }
    else
    {
        float temp = (ed - LOWER_ED_LIMIT) * (float)0xFF /
                     (float)(UPPER_ED_LIMIT - LOWER_ED_LIMIT);
        result = (uint8_t)temp;
    }

    return result;
}


#if (MAC_SCAN_ED_REQUEST_CONFIRM == 1)
/**
 * @brief Stops ED Scan
 *
 * This function stops an ED Scan and completes it by calling tal_ed_end_cb().
 *
 * @param trx_id Transceiver identifier
 */
void stop_ed_scan(trx_id_t trx_id)
{
    /* Stop continuous energy detection */
    CALC_REG_OFFSET(trx_id);
    pal_dev_bit_write(RF215_TRX, GET_REG_ADDR(SR_RF09_EDC_EDM), RF_EDAUTO);
    sampler_counter[trx_id] = 0;
    /* Clear any pending ED IRQ */
    TAL_RF_IRQ_CLR(trx_id, RF_IRQ_EDC);
    handle_ed_end_irq(trx_id);
}
#endif


/* EOF */
