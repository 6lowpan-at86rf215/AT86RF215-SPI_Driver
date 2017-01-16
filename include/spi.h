#ifndef _SPI_H
#define _SPI_H

#include <stdint.h>
#include <unistd.h>

struct spi_t{
	char *name;
	uint8_t mode;
	uint8_t bits;
	uint32_t speed;
	uint16_t delay;
	int fd;
};

struct spi_data_t{
	uint16_t address;
	uint8_t* data;
	uint32_t len;
};

int spi_init(struct spi_t* spi);
int spi_write(struct spi_t* spi,struct spi_data_t* data);
int spi_read(struct spi_t* spi,struct spi_data_t *data);
uint8_t spi_reg_read(struct spi_t* spi,uint16_t address);
int spi_reg_write(struct spi_t* spi,uint16_t address,uint8_t value);
uint8_t spi_reg_bit_read(struct spi_t* spi,uint16_t address,uint8_t mask,uint8_t pos);
uint8_t spi_reg_bit_write(struct spi_t* spi,uint16_t address,uint8_t mask,uint8_t pos,uint8_t new_value);


#endif
