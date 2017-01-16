#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "at86rf215_conf.h"
#include "tal.h"
#include "tal_internal.h"
#include "spi.h"
#include "tal_config.h"

extern rf_cmd_state_t trx_state[NUM_TRX];

static retval_t trx_reset(struct At86rf215_Dev_t*at86rf215_dev);
static void trx_init(struct At86rf215_Dev_t*at86rf215_dev);


retval_t tal_init(struct At86rf215_Dev_t*at86rf215_dev){
	retval_t status=MAC_SUCCESS;
	
	if(trx_reset(at86rf215_dev)!=MAC_SUCCESS){
		return FAILURE;
	}
	/* Check if RF215 is connected */
	if ((spi_reg_read(at86rf215_dev->spi, RG_RF_PN) != 0x34) ||	(spi_reg_read(at86rf215_dev->spi, RG_RF_VN) != 0x03))
	{
		return FAILURE;
	}
	trx_init(at86rf215_dev);



	return status;
}

static retval_t trx_reset(struct At86rf215_Dev_t*at86rf215_dev){
	retval_t status = MAC_SUCCESS;
	int i=0;
	TAL_RF_IRQ_CLR_ALL(RF09);
	TAL_RF_IRQ_CLR_ALL(RF24);

	set_gpio_value(at86rf215_dev->gpio_rest,low);
	usleep(1);
	set_gpio_value(at86rf215_dev->gpio_rest,high);
	//此处先忽略irp_pin拉高表示rf215正常启动的判断
	/*
	
	*/
	for (i = (trx_id_t)0; i < NUM_TRX; i++)
	{
		CALC_REG_OFFSET(i);
		trx_state[i] = (rf_cmd_state_t)spi_reg_read(at86rf215_dev->spi,GET_REG_ADDR(RG_RF09_STATE));
		if (trx_state[i] != RF_TRXOFF)
		{
			status = FAILURE;
		}
	}
	trx_irq_handler_cb(at86rf215_dev);
    for (i = (trx_id_t)0; i < NUM_TRX; i++)
    {
        TAL_RF_IRQ_CLR(i, RF_IRQ_WAKEUP);
    }
	//pal_dev_irq_flag_clr(RF215_TRX);//清理硬件中断

	return status;
}

static void trx_init(struct At86rf215_Dev_t*at86rf215_dev)
{
    /*
     * Configure generic trx functionality
     * Configure Trx registers that are reset during DEEP_SLEEP and IC reset
     * I.e.: RF_CFG, RF_BMDVC, RF_XOC, RF_IQIFC0, RF_IQIFC1, RF_IQIFC2
     */
#ifdef TRX_IRQ_POLARITY
#if (TRX_IRQ_POLARITY == 1)
    //pal_dev_bit_write(RF215_TRX, SR_RF_CFG_IRQP, 1); /* 1 = active low */
    spi_reg_bit_write(at86rf215_dev->spi, SR_RF_CFG_IRQP, 1);
#endif
#endif
#if (TRX_CLOCK_OUTPUT_SELECTION != 1)
    //pal_dev_bit_write(RF215_TRX, SR_RF_CLKO_OS, TRX_CLOCK_OUTPUT_SELECTION);
    spi_reg_bit_write(at86rf215_dev->spi, SR_RF_CLKO_OS, TRX_CLOCK_OUTPUT_SELECTION);
#endif
}

