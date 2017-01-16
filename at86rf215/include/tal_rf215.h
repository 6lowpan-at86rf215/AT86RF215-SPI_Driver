#ifndef TAL_RF215_H
#define TAL_RF215_H
/* === EXTERNALS =========================================================== */
#include "at86rf215.h"

extern volatile bb_irq_t tal_bb_irqs[NUM_TRX];
extern volatile rf_irq_t tal_rf_irqs[NUM_TRX];
/* === MACROS ============================================================== */

/** Clear all interrupts for provided baseband core */
#define TAL_BB_IRQ_CLR_ALL(BB_CORE)     tal_bb_irqs[BB_CORE] = BB_IRQ_NO_IRQ
/** Check if certain IRQ is set for provided baseband core */
#define TAL_BB_IS_IRQ_SET(BB_CORE, IRQ) ((tal_bb_irqs[BB_CORE] & IRQ) ? 1 : 0)
/** Clear interrupt(s) for provided baseband core, avoid Pa091 */
#define TAL_BB_IRQ_CLR(BB_CORE, IRQ)    tal_bb_irqs[BB_CORE] &= (uint8_t)(~((uint32_t)IRQ))
/** Add interrupt(s) for provided baseband core */
#define TAL_BB_IRQ_ADD(BB_CORE, IRQ)    tal_bb_irqs[BB_CORE] |= IRQ

/** Clear all interrupts for provided radio core */
#define TAL_RF_IRQ_CLR_ALL(RF_CORE)     tal_rf_irqs[RF_CORE] = RF_IRQ_NO_IRQ
/** Check if certain IRQ is set for provided radio core */
#define TAL_RF_IS_IRQ_SET(RF_CORE, IRQ) ((tal_rf_irqs[RF_CORE] & IRQ) ? 1 : 0)
/** Clear interrupt(s) for provided radio core,  avoid Pa091 */
#define TAL_RF_IRQ_CLR(RF_CORE, IRQ)    tal_rf_irqs[RF_CORE] &= (uint8_t)(~((uint32_t)IRQ))
/** Add interrupt(s) for provided radio core */
#define TAL_RF_IRQ_ADD(RF_CORE, IRQ)    tal_rf_irqs[RF_CORE] |= IRQ


/** Register offset between RF09 and TRX24 */
#define RF_BASE_ADDR_OFFSET             (BASE_ADDR_RF24_RF24 - BASE_ADDR_RF09_RF09)
/** Register offset between the baseband cores */
#define BB_BASE_ADDR_OFFSET             (BASE_ADDR_BBC1_CORE1 - BASE_ADDR_BBC0_CORE0)
/** Register offset between the Tx frame buffers. */
#define BB_TX_FRM_BUF_OFFSET            (RG_BBC1_FBTXS - RG_BBC0_FBTXS)
/** Register offset between the Rx frame buffers. */
#define BB_RX_FRM_BUF_OFFSET            (RG_BBC1_FBRXS - RG_BBC0_FBRXS)
/** Register offset for test register area. */
#define RFT_TST_ADDR_OFFSET             (0x280 - 0x180)

/** Clear all interrupts for provided radio core */
#define TAL_RF_IRQ_CLR_ALL(RF_CORE)     tal_rf_irqs[RF_CORE] = RF_IRQ_NO_IRQ


#endif

