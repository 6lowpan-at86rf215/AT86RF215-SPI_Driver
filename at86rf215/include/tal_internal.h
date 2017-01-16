#ifndef TAL_INTERNAL_H
#define TAL_INTERNAL_H
#include "tal_rf215.h"
#include "at86rf215_conf.h"

#define CALC_REG_OFFSET(var)                uint16_t offset = RF_BASE_ADDR_OFFSET * var
#define GET_REG_ADDR(reg)                   offset + reg
typedef enum tal_state_tag
{
    TAL_IDLE,
    TAL_SLEEP,
    TAL_RESET,
    TAL_WAKING_UP,
    TAL_TX,
    TAL_ED_SCAN
#if (defined SUPPORT_TFA) || (defined TFA_CCA) || (defined TFA_CW)
    ,
    TAL_TFA_CW_RX,
    TAL_TFA_CW,
    TAL_TFA_CCA
#endif
#ifdef SUPPORT_MODE_SWITCH
    ,
    TAL_NEW_MODE_RECEIVING,
    TAL_ACK_TRANSMITTING
#endif
}  tal_state_t;

extern tal_state_t tal_state[NUM_TRX];
extern uint32_t rxe_txe_tstamp[NUM_TRX];


void trx_irq_handler_cb(struct At86rf215_Dev_t*at86rf215_dev);



#endif

