/**
 * @file phy_conf.c
 *
 * @brief  PHY configuration
 *
 * $Id: phy_conf.c 37522 2015-06-16 14:52:55Z uwalter $
 *
 * @author    Atmel Corporation: http://www.atmel.com
 * @author    Support email: avr@atmel.com
 */
/*
 * Copyright (c) 2014, Atmel Corporation All rights reserved.
 *
 * Licensed under Atmel's Limited License Agreement --> EULA.txt
 */

/* === INCLUDES ============================================================ */

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <ctype.h>
#include "pal.h"
#include "tal.h"
#include "app_config.h"
//#include "sio_handler.h"
#include "app_common.h"
#include "ieee_154g.h"

/* === TYPES =============================================================== */

/* === MACROS ============================================================== */

/* === EXTERNALS =========================================================== */

/* === GLOBALS ============================================================= */

/* === PROTOTYPES ========================================================== */

/* === IMPLEMENTATION ====================================================== */


#ifdef MULTI_TRX_SUPPORT
/**
 * @brief Set modulation
 *
 * @param trx_id    Transceiver identifier
 * @param mod       Modulation
 *
 * @return          MAC_SUCCESS or FAILURE
 */
retval_t set_mod(trx_id_t trx_id, modulation_t mod)
{
    retval_t status;

    switch (mod)
    {
        case FSK:
            status = set_fsk(trx_id);
            break;

        case OFDM:
            status = set_ofdm(trx_id);
            break;

        case OQPSK:
            status = set_oqpsk(trx_id);
            break;

        case LEG_OQPSK:
            status = set_leg_oqpsk(trx_id);
            break;

        default:
            status = FAILURE;
#ifdef USER_CONFIGURATION
			if (USER_CONFIG == (uint8_t)mod)
			{
				status = set_user_config(trx_id);
			} 
#endif /*USER_CONFIGURATION*/
    }

    if (status == MAC_SUCCESS)
    {
        current_mod[trx_id] = mod;
    }
    return status;
}
#endif /* #ifdef MULTI_TRX_SUPPORT */


#ifdef MULTI_TRX_SUPPORT
/**
 * @brief Set FSK modulation
 *
 * @param trx_id    Transceiver identifier
 *
 * @return          MAC_SUCCESS or FAILURE
 */
retval_t set_fsk(trx_id_t trx_id)
{
    phy_t phy;
    retval_t status;
    int pwr;

    phy.modulation = FSK;
    phy.phy_mode.fsk.sym_rate = FSK_SYM_RATE_50;
    phy.phy_mode.fsk.mod_idx = MOD_IDX_1_0;
    phy.phy_mode.fsk.mod_type = F2FSK;
    if (trx_id == RF09)
    {
        phy.freq_band = US_915;
        phy.ch_spacing = FSK_915_MOD1_CH_SPAC;
        phy.freq_f0 = FSK_915_MOD1_F0;
        pwr = 14;
    }
    else // RF24
    {
        phy.freq_band = WORLD_2450;
        phy.ch_spacing = FSK_2450_MOD1_CH_SPAC;
        phy.freq_f0 = FSK_2450_MOD1_F0;
#ifdef RF215v1
        pwr = 12;
#else
        pwr = 14;
#endif
    }

    /* Set preamble length */
    uint16_t len = 8;
    status = tal_pib_set(trx_id, phyFSKPreambleLength, (pib_value_t *)&len);
    if (status != MAC_SUCCESS)
    {
        return status;
    }

    /* Set FEC */
    bool fec = false;
    status = tal_pib_set(trx_id, phyFSKFECEnabled, (pib_value_t *)&fec);
    if (status != MAC_SUCCESS)
    {
        return status;
    }

    /* Data whitening */
    bool dw = true;
    status = tal_pib_set(trx_id, phyFSKScramblePSDU, (pib_value_t *)&dw);
    if (status != MAC_SUCCESS)
    {
        return status;
    }

    /* CRC */
    uint16_t crc_type = FCS_TYPE_4_OCTETS;
    status = tal_pib_set(trx_id, macFCSType, (pib_value_t *)&crc_type);
    if (status != MAC_SUCCESS)
    {
        return status;
    }

    /* Set modulation / PHY configuration */
    status = tal_pib_set(trx_id, phySetting, (pib_value_t *)&phy);
    if (status != MAC_SUCCESS)
    {
        return status;
    }

    /* Set channel */
    channel_t ch = 0;
    status = tal_pib_set(trx_id, phyCurrentChannel, (pib_value_t *)&ch);
    if (status != MAC_SUCCESS)
    {
        return status;
    }

    /* Set transmit power */
    status = tal_pib_set(trx_id, phyTransmitPower, (pib_value_t *)&pwr);
    if (status != MAC_SUCCESS)
    {
        return status;
    }

    /* Enable RPC */
    bool rpc = true;
    status = tal_pib_set(trx_id, phyRPCEnabled, (pib_value_t *)&rpc);
    if (status != MAC_SUCCESS)
    {
        return status;
    }

    return MAC_SUCCESS;
}
#endif /* #ifdef MULTI_TRX_SUPPORT */


#ifdef MULTI_TRX_SUPPORT
/**
 * @brief Set OFDM modulation
 *
 * @param trx_id    Transceiver identifier
 *
 * @return          MAC_SUCCESS or FAILURE
 */
retval_t set_ofdm(trx_id_t trx_id)
{
    phy_t phy;
    retval_t status;

    phy.modulation = OFDM;
    phy.phy_mode.ofdm.option = OFDM_OPT_1;
    if (trx_id == RF09)
    {
        phy.freq_band = US_915;
        phy.ch_spacing = OFDM_915_OPT1_CH_SPAC;
        phy.freq_f0 = OFDM_915_OPT1_F0;
    }
    else // RF24
    {
        phy.freq_band = WORLD_2450;
        phy.ch_spacing = OFDM_2450_OPT1_CH_SPAC;
        phy.freq_f0 = OFDM_2450_OPT1_F0;
    }

    /* Set interleaving */
    bool interl = false;
    status = tal_pib_set(trx_id, phyOFDMInterleaving, (pib_value_t *)&interl);
    if (status != MAC_SUCCESS)
    {
        return status;
    }

    /* Set data rate / MCS */
    ofdm_mcs_t mcs = MCS3;
    status = tal_pib_set(trx_id, phyOFDMMCS, (pib_value_t *)&mcs);
    if (status != MAC_SUCCESS)
    {
        return status;
    }

    /* CRC */
    uint16_t crc_type = FCS_TYPE_4_OCTETS;
    status = tal_pib_set(trx_id, macFCSType, (pib_value_t *)&crc_type);
    if (status != MAC_SUCCESS)
    {
        return status;
    }

    /* Set modulation / PHY configuration */
    status = tal_pib_set(trx_id, phySetting, (pib_value_t *)&phy);
    if (status != MAC_SUCCESS)
    {
        return status;
    }

    /* Set channel */
    channel_t ch = 0;
    status = tal_pib_set(trx_id, phyCurrentChannel, (pib_value_t *)&ch);
    if (status != MAC_SUCCESS)
    {
        return status;
    }

    /* Set transmit power */
    int pwr = 14;
    status = tal_pib_set(trx_id, phyTransmitPower, (pib_value_t *)&pwr);
    if (status != MAC_SUCCESS)
    {
        return status;
    }

    return MAC_SUCCESS;
}
#endif /* #ifdef MULTI_TRX_SUPPORT */


#ifdef MULTI_TRX_SUPPORT
/**
 * @brief Set MR-OQPSK modulation
 *
 * @param trx_id    Transceiver identifier
 *
 * @return          MAC_SUCCESS or FAILURE
 */
retval_t set_oqpsk(trx_id_t trx_id)
{
    phy_t phy;
    retval_t status;

    phy.modulation = OQPSK;
    phy.phy_mode.oqpsk.chip_rate = CHIP_RATE_100;
    if (trx_id == RF09)
    {
        phy.freq_band = US_915;
        phy.ch_spacing = OQPSK_915_CH_SPAC;
        phy.freq_f0 = OQPSK_915_F0;
    }
    else
    {
        phy.freq_band = WORLD_2450;
        phy.ch_spacing = OQPSK_2450_CH_SPAC;
        phy.freq_f0 = OQPSK_2450_F0;
    }

    /* Set data rate / rate mode */
    oqpsk_rate_mode_t rate = OQPSK_RATE_MOD_0;
    status = tal_pib_set(trx_id, phyOQPSKRateMode, (pib_value_t *)&rate);
    if (status != MAC_SUCCESS)
    {
        return status;
    }

    /* CRC */
    uint16_t crc_type = FCS_TYPE_4_OCTETS;
    status = tal_pib_set(trx_id, macFCSType, (pib_value_t *)&crc_type);
    if (status != MAC_SUCCESS)
    {
        return status;
    }

    /* Set modulation / PHY configuration */
    status = tal_pib_set(trx_id, phySetting, (pib_value_t *)&phy);
    if (status != MAC_SUCCESS)
    {
        return status;
    }

    /* Set channel */
    channel_t ch = 0;
    status = tal_pib_set(trx_id, phyCurrentChannel, (pib_value_t *)&ch);
    if (status != MAC_SUCCESS)
    {
        return status;
    }

    /* Set transmit power */
    int pwr = 14;
    status = tal_pib_set(trx_id, phyTransmitPower, (pib_value_t *)&pwr);
    if (status != MAC_SUCCESS)
    {
        return status;
    }

    /* Enable RPC */
    bool rpc = true;
    status = tal_pib_set(trx_id, phyRPCEnabled, (pib_value_t *)&rpc);
    if (status != MAC_SUCCESS)
    {
        return status;
    }

    return MAC_SUCCESS;
}
#endif /* #ifdef MULTI_TRX_SUPPORT */


#ifdef MULTI_TRX_SUPPORT
/**
 * @brief Set legacy OQPSK modulation
 *
 * @param trx_id    Transceiver identifier
 *
 * @return          MAC_SUCCESS or FAILURE
 */
retval_t set_leg_oqpsk(trx_id_t trx_id)
{
    phy_t phy;
    retval_t status;
    channel_t ch;

    phy.modulation = LEG_OQPSK;
    if (trx_id == RF09)
    {
        phy.phy_mode.leg_oqpsk.chip_rate = CHIP_RATE_1000;
        phy.freq_band = US_915;
        phy.ch_spacing = LEG_915_CH_SPAC;
        phy.freq_f0 = LEG_915_F0 - LEG_915_CH_SPAC;
        ch = 1;
    }
    else // RF24
    {
        phy.phy_mode.leg_oqpsk.chip_rate = CHIP_RATE_2000;
        phy.freq_band = WORLD_2450;
        phy.ch_spacing = LEG_2450_CH_SPAC;
        phy.freq_f0 = LEG_2450_F0 - (11 * LEG_2450_CH_SPAC);
        ch = 11;
    }

    /* Set modulation / PHY configuration */
    status = tal_pib_set(trx_id, phySetting, (pib_value_t *)&phy);
    if (status != MAC_SUCCESS)
    {
        return status;
    }

    /* Set channel */
    status = tal_pib_set(trx_id, phyCurrentChannel, (pib_value_t *)&ch);
    if (status != MAC_SUCCESS)
    {
        return status;
    }

    /* Set transmit power */
    int pwr = 14;
    status = tal_pib_set(trx_id, phyTransmitPower, (pib_value_t *)&pwr);
    if (status != MAC_SUCCESS)
    {
        return status;
    }

    return MAC_SUCCESS;
}

#ifdef USER_CONFIGURATION 
/**
 * @brief Set User config
 *
 * @param trx_id    Transceiver identifier
 *
 * @return          MAC_SUCCESS or FAILURE
 */
retval_t set_user_config(trx_id_t trx_id)
{
	printf("\n\rThe User can add their custom configuration in phy_conf.c -> set_user_config()");
	return MAC_SUCCESS;
}
#endif /*#ifdef USER_CONFIGURATION*/ 
#endif /* #ifdef MULTI_TRX_SUPPORT */



/* EOF */
