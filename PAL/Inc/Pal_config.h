#ifndef PAL_CONFIG_H
#define PAL_CONFIG_H
#include <unistd.h>

/*
 * This macro is used to mark the start of a critical region. It saves the
 * current status register and then disables the interrupt.
 */
#define ENTER_CRITICAL_REGION()

/*
 * This macro is used to mark the end of a critical region. It restores the
 * current status register.
 */
#define LEAVE_CRITICAL_REGION()

/*
 * This macro saves the trx interrupt status and disables the trx interrupt.
 */
#define ENTER_TRX_REGION()

/*
 *  This macro restores the transceiver interrupt status
 */
#define LEAVE_TRX_REGION()


/*
 * This board uses an SPI-attached transceiver.
 */
#define PAL_USE_SPI_TRX                 (1)


/**
 * Multiple transceivers are supported
 */
#define MULTI_TRX_SUPPORT


/**
 * Attached transceiver uses SPI block mode
 
 */
#define PAL_SPI_BLOCK_MODE


#define PAL_WAIT_1_US()					usleep(1)

#endif

