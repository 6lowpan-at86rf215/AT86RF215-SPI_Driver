#ifndef _SPI_H
#define _SPI_H
struct spi_t{
	char *name;
	uint8_t mode;
	uint8_t bits;
	uint32_t speed;
	uint16_t delay;
	int fd;
};
void spi_init(void);
void transfer(uint8_t*tx,uint32_t tx_len,uint8_t*rx,uint32_t rx_len);
#endif
