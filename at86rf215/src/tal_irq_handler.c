#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "tal_internal.h"
#include "spi.h"
#include "at86rf215_conf.h"
#include "at86rf215.h"
#include "tal.h"
/**
 * @brief Transceiver interrupt handler
 *
 * This function handles the transceiver interrupt. It reads all IRQs from the
 * transceivers and stores them to a variable. If a transceiver is currently
 * sleeping, then the IRQs are not handled.
 * The actual processing of the IRQs is triggered from tal_task().
 */
void trx_irq_handler_cb(struct At86rf215_Dev_t*at86rf215_dev)
{

    /* Get all IRQS values */
    uint8_t irqs_array[4];
	trx_id_t trx_id;
	int i;
    //pal_dev_read(RF215_TRX, RG_RF09_IRQS, irqs_array, 4);
    struct spi_data_t message={
		.address=RG_RF09_IRQS,
		.data=irqs_array,
		.len=sizeof(irqs_array)/sizeof(uint8_t)
	};
	spi_read(at86rf215_dev->spi,&message);
/*
	printf("recv mesage:");
	for(i=0;i<message.len;i++)
		printf("%.2X ", message.data[i]);
	printf("\n");
*/
    /* Handle BB IRQS */
    for (trx_id = (trx_id_t)0; trx_id < NUM_TRX; trx_id++)
    {
        if (tal_state[trx_id] == TAL_SLEEP)
        {
            continue;
        }

        bb_irq_t irqs = (bb_irq_t)irqs_array[trx_id + 2];

        if (irqs != BB_IRQ_NO_IRQ)
        {
            if (irqs & BB_IRQ_RXEM)
            {
                irqs &= (uint8_t)(~((uint32_t)BB_IRQ_RXEM)); // avoid Pa091
            }
            if (irqs & BB_IRQ_RXAM)
            {
                irqs &= (uint8_t)(~((uint32_t)BB_IRQ_RXAM)); // avoid Pa091
            }
            if (irqs & BB_IRQ_AGCR)
            {
                irqs &= (uint8_t)(~((uint32_t)BB_IRQ_AGCR)); // avoid Pa091
            }
            if (irqs & BB_IRQ_AGCH)
            {
                irqs &= (uint8_t)(~((uint32_t)BB_IRQ_AGCH)); // avoid Pa091
            }
            if (irqs & BB_IRQ_RXFS)
            {
                irqs &= (uint8_t)(~((uint32_t)BB_IRQ_RXFS)); // avoid Pa091
            }
            if (irqs & BB_IRQ_RXFE)
            {
                //pal_get_current_time(&rxe_txe_tstamp[trx_id]);

            }
            if (irqs & BB_IRQ_TXFE)
            {
                /* used for IFS and for MEASURE_ON_AIR_DURATION */
                //pal_get_current_time(&rxe_txe_tstamp[trx_id]);
            }

            /*
             * Store remaining flags to global TAL variable and
             * handle them within tal_task()
             */
            tal_bb_irqs[trx_id] |= irqs;
        }
    }

    /* Handle RF IRQS */
    for (trx_id = (trx_id_t)0; trx_id < NUM_TRX; trx_id++)
    {
        if (tal_state[trx_id] == TAL_SLEEP)
        {
            continue;
        }

        rf_irq_t irqs = (rf_irq_t)irqs_array[trx_id];

        if (irqs != RF_IRQ_NO_IRQ)
        {
            if (irqs & RF_IRQ_TRXRDY)
            {
                irqs &= (uint8_t)(~((uint32_t)RF_IRQ_TRXRDY)); // avoid Pa091
            }
            if (irqs & RF_IRQ_TRXERR)
            {
                irqs &= (uint8_t)(~((uint32_t)RF_IRQ_TRXERR)); // avoid Pa091
            }
            tal_rf_irqs[trx_id] |= irqs;
        }
    }
}/* trx_irq_handler_cb() */

