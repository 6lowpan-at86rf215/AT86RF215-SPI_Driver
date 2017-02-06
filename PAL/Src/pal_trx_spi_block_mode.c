
#include <stdbool.h>
#include <stdint.h>
#include "pal.h"
#include "pal_types.h"

#if (defined PAL_SPI_BLOCK_MODE) || (defined DOXYGEN)

/* === Externals ============================================================ */
extern At86rf215_Dev_t at86rf215_dev;

/* === Macros =============================================================== */


void pal_trx_write(uint16_t addr, uint8_t *data, uint16_t length)
{
	spi_data_t message={
		.address=addr,
		.data=data,
		.len=length
	};
	spi_write(at86rf215_dev.spi,&message);
}


void pal_trx_read(uint16_t addr, uint8_t *data, uint16_t length)
{
	spi_data_t message={
		.address=addr,
		.data=data,
		.len=length
	};
	spi_read(at86rf215_dev.spi,&message);
}


void pal_trx_reg_write(uint16_t addr, uint8_t data)
{
	spi_reg_write(at86rf215_dev.spi,addr,data);
}


uint8_t pal_trx_reg_read(uint16_t addr)
{
	return spi_reg_read(at86rf215_dev.spi,addr);
}

uint8_t pal_trx_bit_read(uint16_t addr, uint8_t mask, uint8_t pos){
	uint8_t ret=spi_reg_bit_read(at86rf215_dev.spi,addr,mask,pos);
	return ret;

} 

 
void pal_trx_bit_write(uint16_t addr, uint8_t mask, uint8_t pos, uint8_t new_value) 
{
 	spi_reg_bit_write(at86rf215_dev.spi,addr,mask,pos,new_value);
}



#endif  /* #if (defined PAL_SPI_BLOCK_MODE) || (defined DOXYGEN) */


/* EOF */
