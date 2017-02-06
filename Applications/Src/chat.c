/**
 * @file chat.c
 *
 * @brief  Chat feature handling
 *
 * $Id: chat.c 37588 2015-06-23 09:40:22Z uwalter $
 *
 * @author    Atmel Corporation: http://www.atmel.com
 * @author    Support email: avr@atmel.com
 */
/*
 * Copyright (c) 2013, Atmel Corporation All rights reserved.
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
#include "tal.h"
#include "app_config.h"
#include "app_common.h"

/* === TYPES =============================================================== */

/* === MACROS ============================================================== */

/* === EXTERNALS =========================================================== */

/* === GLOBALS ============================================================= */

frame_info_t *tx_frame;
uint8_t tx_buf[LARGE_BUFFER_SIZE];
uint8_t *tx_frm_pay_ptr;
uint8_t *chat_pay_ptr;
uint8_t input_len;
uint8_t tx_hdr_len;
#ifdef MAC_2014
uint8_t crc_len[2];
#endif

/* === PROTOTYPES ========================================================== */

/* === IMPLEMENTATION ====================================================== */


/**
 * @brief Print chat welcome menu
 */
void print_chat_menu(void)
{
    CLEAR_SCREEN();

#ifdef MAC_2014
    for (trx_id_t i = (trx_id_t)0; i < NUM_TRX; i++)
    {
        uint8_t fcs_type;
        tal_pib_get(i, macFCSType, &fcs_type);
        if (fcs_type == FCS_TYPE_4_OCTETS)
        {
            crc_len[i] = 4;
        }
        else
        {
            crc_len[i] = 2;
        }
    }
#endif

    printf("Chat Application\n%s, %s\n",get_tal_type_text(TAL_TYPE),get_pal_type_text(PAL_TYPE));
           //get_board_text(BOARD_TYPE), __DATE__)
           ;
#ifdef MULTI_TRX_SUPPORT
    printf("Active transmitter: %s, ", get_trx_id_text(current_trx_id));
    if (sleep_enabled[current_trx_id])
    {
        printf("off\n");
    }
    else
    {
        printf("%s\n", get_mod_text(current_mod[current_trx_id]));
    }

    printf("Press 'CTRL-s' for sub-1GHz or 'CTRL-d' for 2.4GHz.\n");
    printf("Press '/' to enter configuration menu.\n");
#endif

    printf("Enter text and press 'enter' to trigger transmission\n");
    printf("\n");
    printf("> ");
    fflush(stdout);
}


/**
 * @brief Initialize transmit frame header
 */
void init_tx_frame(void)
{
    uint8_t *ptr;

    tx_frame = (frame_info_t *)tx_buf;
#ifdef MAC_2014
    tx_frame->mpdu = (uint8_t *)tx_frame + LARGE_BUFFER_SIZE - aMaxPHYPacketSize_4g;
#else
    tx_frame->mpdu = (uint8_t *)tx_frame + LARGE_BUFFER_SIZE - aMaxPHYPacketSize - 1; // 1 = length field
#endif
    ptr = &tx_frame->mpdu[PL_POS_FCF_1]; // start with fcf; length is set elsewhere
    /* fcf0 */
    *ptr = FCF_FRAMETYPE_DATA;
#if (PEER_ACK_REQUEST == true)
    *ptr |= FCF_ACK_REQUEST;
#endif
    ptr++;
    /* fcf1: use short address mode */
    uint8_t dest_addr_mode = DEST_ADDR_MODE;
    uint8_t src_addr_mode = SRC_ADDR_MODE;
    *ptr++ = (dest_addr_mode << FCF_2_DEST_ADDR_OFFSET) | (src_addr_mode << FCF_2_SOURCE_ADDR_OFFSET);
    *ptr++ = (uint8_t)rand(); // seq_no
    if (dest_addr_mode == FCF_SHORT_ADDR)
    {
        uint16_t dest_pan_id = PEER_PAN_ID;
        memcpy(ptr, &dest_pan_id, 2);
        ptr += 2;
        uint16_t dest_short_addr = PEER_SHORT_ADDR;
        memcpy(ptr, &dest_short_addr, 2);
        ptr += 2;
    }
    else if (dest_addr_mode == FCF_LONG_ADDR)
    {
        uint16_t dest_pan_id = PEER_PAN_ID;
        memcpy(ptr, &dest_pan_id, 2);
        ptr += 2;
        uint64_t peer_ieee_addr = PEER_IEEE_ADDR;
        memcpy(ptr, &peer_ieee_addr, 8);
        ptr += 8;
    }

    if (src_addr_mode == FCF_SHORT_ADDR)
    {
        uint16_t src_pan_id;
        uint16_t src_short_addr;
        tal_pib_get(current_trx_id, macPANId, (uint8_t *)&src_pan_id);
        tal_pib_get(current_trx_id, macShortAddress, (uint8_t *)&src_short_addr);
        memcpy(ptr, &src_pan_id, 2);
        ptr += 2;
        memcpy(ptr, &src_short_addr, 2);
        ptr += 2;
    }
    else if (src_addr_mode == FCF_LONG_ADDR)
    {
        uint16_t src_pan_id;
        uint8_t ieee_addr[8];
        tal_pib_get(current_trx_id, macPANId, (uint8_t *)&src_pan_id);
        tal_pib_get(current_trx_id, macIeeeAddress, ieee_addr);
        memcpy(ptr, &src_pan_id, 2);
        ptr += 2;
        memcpy(ptr, &ieee_addr, 8);
        ptr += 8;
    }
    tx_hdr_len = ptr - tx_frame->mpdu;
    tx_frm_pay_ptr = ptr;
    chat_pay_ptr = tx_frm_pay_ptr;
}


/**
 * @brief Handle incoming frame
 *
 * @param trx_id Transceiver identifier
 * @param rx_frame Pointer to frame_info_t structure
 */
void chat_handle_incoming_frame(trx_id_t trx_id, frame_info_t *rx_frame)
{
    uint8_t hdr_len;
    uint8_t *pay_ptr;

#ifdef MAC_2014
    uint16_t pay_len;
    uint8_t src_addr_mode = (rx_frame->mpdu[PL_POS_FCF_2] >> 6) & 0x03;
    uint8_t dest_addr_mode = (rx_frame->mpdu[PL_POS_FCF_2] >> 2) & 0x03;
    bool pan_id_comp = (rx_frame->mpdu[PL_POS_FCF_1] >> 6) & 0x01; // only dest pan id
    pay_ptr = (uint8_t *)&rx_frame->mpdu[0] + 3;
    int8_t dbm = rx_frame->mpdu[rx_frame->len_no_crc + 1 + crc_len[trx_id]];
    printf("Rx (%s, %"PRIi8"dBm, from 0x", get_trx_id_text(trx_id), dbm);
#else
    uint8_t pay_len;
    uint8_t src_addr_mode = (rx_frame->mpdu[PL_POS_FCF_2] >> 6) & 0x03;
    uint8_t dest_addr_mode = (rx_frame->mpdu[PL_POS_FCF_2] >> 2) & 0x03;
    bool pan_id_comp = (rx_frame->mpdu[PL_POS_FCF_1] >> 6) & 0x01; // only dest pan id
    pay_ptr = (uint8_t *)&rx_frame->mpdu[0] + 4;
    printf("Rx (from 0x");
#endif

    if (dest_addr_mode == FCF_SHORT_ADDR)
    {
        pay_ptr += 4; // short addr and pan id
    }
    else if (dest_addr_mode == FCF_LONG_ADDR)
    {
        pay_ptr += 10; // long addr and pan id
    }

    if (!pan_id_comp)
    {
        pay_ptr += 2; // src pan id
    }

    if (src_addr_mode == FCF_SHORT_ADDR)
    {
        uint16_t src_addr;
        memcpy(&src_addr, pay_ptr, 2);
        printf("%.4"PRIX16"", src_addr);
        pay_ptr += 2; // short addr
    }
    else if (src_addr_mode == FCF_LONG_ADDR)
    {
        uint8_t src_addr[8];
        memcpy(src_addr, pay_ptr, 8);
        for (uint8_t i = 0; i < 8; i++)
        {
            printf("%.2"PRIX8"", src_addr[7 - i]);
        }
        pay_ptr += 8; // long addr
    }
    printf("): ");

#ifdef MAC_2014
    hdr_len = pay_ptr - &rx_frame->mpdu[0];
    pay_len = rx_frame->len_no_crc - hdr_len;
#else
    hdr_len = pay_ptr - &rx_frame->mpdu[0] - 1;
    pay_len = rx_frame->mpdu[0] - 2 - hdr_len;
#endif

	if (pay_len > MAX_INPUT_LENGTH)
	{
		pay_len = 0;
	}

    for (uint16_t i = 0; i < pay_len; i++)
    {
        if (isprint(*pay_ptr))
        {
            printf("%c", *pay_ptr);
        }
        else
        {
            printf(" ");
        }
        pay_ptr++;
    }

    printf("\n");
    printf("> ");
    fflush(stdout);

    /* Keep compiler happy */
    trx_id = trx_id;
}


/**
 * @brief Get input character and store it to buffer. If frame is completed,
 *        transmit it.
 *
 * @param input Input character
 */
void get_chat_input(char input)
{
    if (app_state[current_trx_id] == APP_IDLE)
    {
		if (sleep_enabled[current_trx_id])
		{
			printf("\nStatus: Transceiver in Sleep State");
			printf("\nPress '/' to enter configuration menu.\n");
			printf("\n> ");
			fflush(stdout);
			return;
		}
		if(input == '\b')
		{
			input_len--;
			chat_pay_ptr--;
			return;
		}	
        *chat_pay_ptr++ = input;	
        input_len++;
        if ((input == NL) || (input == CR) || (input_len == MAX_INPUT_LENGTH))
        {
            app_state[current_trx_id] = APP_TX;
#ifdef MAC_2014
            tx_frame->len_no_crc = input_len + tx_hdr_len;
#else
            tx_frame->mpdu[0] = input_len + tx_hdr_len + 1;
#endif
            tx_frame->mpdu[PL_POS_SEQ_NUM]++;

            tal_tx_frame(current_trx_id, tx_frame, CSMA_MODE, RETRANSMISSION_ENABLED);
            input_len = 0;
        }
    }
}


/**
 * @brief Handle transmit done callback
 *
 * @param trx_id Transceiver identifier
 * @param status Status of the transmission
 * @param frame Pointer to frame_info_t structure
 */
void chat_tx_done_cb(trx_id_t trx_id, retval_t status, frame_info_t *frame)
{
    if (status != MAC_SUCCESS)
    {
        printf(STATUS_TEXT);
    }

    printf("\n> ");
    fflush(stdout);

    chat_pay_ptr = tx_frm_pay_ptr;
    input_len = 0;
    app_state[trx_id] = APP_IDLE;

    /* Keep compiler happy */
    frame = frame;
}



/* EOF */
