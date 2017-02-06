/**
 * @file app_common.h
 *
 * @brief  Common application defines, variables and prototypes
 *
 * $Id: app_common.h 37590 2015-06-23 09:41:24Z uwalter $
 *
 * @author    Atmel Corporation: http://www.atmel.com
 * @author    Support email: avr@atmel.com
 */
/*
 * Copyright (c) 2012, Atmel Corporation All rights reserved.
 *
 * Licensed under Atmel's Limited License Agreement --> EULA.txt
 */

/* Prevent double inclusion */
#ifndef APP_COMMON_H
#define APP_COMMON_H

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
#include "ieee_const.h"
#include "bmm.h"
//#include "sio_handler.h"

/* === TYPES =============================================================== */

/* Enumeration used for main state */
typedef enum app_state_tag
{
    APP_IDLE,
    APP_TX
} SHORTENUM app_state_t;


/* === MACROS ============================================================== */

/* Character defines */
#define ESC                     0x1B
#define FORM_FEED               0x0C
#define POS1                    0x5B
#define ENTER                   0x0D
#define NL                      0x0A
#define CR                      0x0D
#define SUB1_CHAR               0x13 /* Ctrl-s */
#define TWO_G_CHAR              0x04 /* Ctrl-d */
#define MENU_CHAR               '/'

/* Default PIB value */
#define OWN_PAN_ID              0xBABE
#define OWN_SHORT_ADDR          0x1234
#define PEER_PAN_ID             OWN_PAN_ID
#define PEER_SHORT_ADDR         0x1235
#define DEST_ADDR_MODE          FCF_SHORT_ADDR // FCF_LONG_ADDR
#define SRC_ADDR_MODE           FCF_SHORT_ADDR // FCF_LONG_ADDR
#define PEER_IEEE_ADDR          0x0807060504030201
#define PEER_ACK_REQUEST        true
#define RETRANSMISSION_ENABLED  true
#define CSMA_MODE               CSMA_UNSLOTTED

/* Print macros */
#define CANCEL_TEXT             "Enter Crtl-C to stop promiscuous mode."
#define ENTER_ANY_KEY_TEXT      "Press any key to continue."
#define RETURN_TO_MENU_TEXT     "Press any key to return to menu."
#ifdef MULTI_TRX_SUPPORT
#define SELECT_MENU_ITEM_TEXT   "Press 's' for RF09 or 'd' for RF24.\n" \
    "Enter number to select menu item: "
#else
#define SELECT_MENU_ITEM_TEXT   "Enter number to select menu item: "
#endif
#define STATUS_TEXT             "\nStatus: 0x%.2"PRIX8", %s", status, get_retval_text(status)
#define CLEAR_SCREEN()          printf("\n%c", FORM_FEED)

#ifdef USER_CONFIGURATION
#define USER_CONFIG			4
#define PRESET_TEXT_COUNT	5
#else
#define PRESET_TEXT_COUNT	4
#endif
/* === EXTERNALS =========================================================== */

/*
 * Externals from chat.c
 */
extern frame_info_t *tx_frame;
extern uint8_t tx_buf[LARGE_BUFFER_SIZE];
extern uint8_t *tx_frm_pay_ptr;
extern uint8_t *chat_pay_ptr;
extern uint8_t tx_hdr_len;
extern uint8_t input_len;

/* === GLOBALS ============================================================= */

extern app_state_t app_state[];
extern trx_id_t current_trx_id;
extern bool sleep_enabled[];
#ifdef MULTI_TRX_SUPPORT
extern modulation_t current_mod[];
#endif

/* === PROTOTYPES ========================================================== */

/*
 * Function prototypes from main.c
 */
char *get_retval_text(retval_t ret);
char get_input(void);
#ifdef MULTI_TRX_SUPPORT
char *get_trx_id_text(trx_id_t id);
char *get_mod_text(modulation_t mod);
#endif
char *get_tal_type_text(uint8_t tal_type);
//char *get_board_text(uint8_t board);
char *get_pal_type_text(uint8_t pal_type);

/*
 * Function prototypes from chat.c
 */
void print_chat_menu(void);
void chat_handle_incoming_frame(trx_id_t trx_id, frame_info_t *rx_frame);
void get_chat_input(char input);
void chat_tx_done_cb(trx_id_t trx_id, retval_t status, frame_info_t *frame);
void init_tx_frame(void);

/*
 * Function prototypes from phy_config.c
 */
#ifdef MULTI_TRX_SUPPORT
retval_t set_mod(trx_id_t trx_id, modulation_t mod);
retval_t set_fsk(trx_id_t trx_id);
retval_t set_ofdm(trx_id_t trx_id);
retval_t set_oqpsk(trx_id_t trx_id);
retval_t set_leg_oqpsk(trx_id_t trx_id);
#ifdef USER_CONFIGURATION 
retval_t set_user_config(trx_id_t trx_id);
#endif
#endif

/* === IMPLEMENTATION ====================================================== */


#endif /* APP_COMMON_H */
/* EOF */
