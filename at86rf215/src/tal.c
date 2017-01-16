#include "at86rf215.h"
#include "tal_internal.h"
#include "at86rf215_conf.h"

/** Current trx state */
volatile bb_irq_t tal_bb_irqs[NUM_TRX] = {BB_IRQ_NO_IRQ, BB_IRQ_NO_IRQ};
volatile rf_irq_t tal_rf_irqs[NUM_TRX] = {RF_IRQ_NO_IRQ, RF_IRQ_NO_IRQ};
tal_state_t tal_state[NUM_TRX];
uint32_t rxe_txe_tstamp[NUM_TRX];


rf_cmd_state_t trx_state[NUM_TRX];

