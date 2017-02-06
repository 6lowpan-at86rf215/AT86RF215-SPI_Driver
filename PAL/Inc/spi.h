#ifndef _SPI_H
#define _SPI_H

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>


typedef struct spi_tag{
	char *name;
	uint8_t mode;
	uint8_t bits;
	uint32_t speed;
	uint16_t delay;
	int fd;
}spi_t;

typedef struct spi_data_tag{
	uint16_t address;
	uint8_t* data;
	uint32_t len;
}spi_data_t;

int spi_init(spi_t* spi);
int spi_write(spi_t* spi,spi_data_t* data);
int spi_read(spi_t* spi,spi_data_t *data);
uint8_t spi_reg_read(spi_t* spi,uint16_t address);
int spi_reg_write(spi_t* spi,uint16_t address,uint8_t value);
uint8_t spi_reg_bit_read(spi_t* spi,uint16_t address,uint8_t mask,uint8_t pos);
uint8_t spi_reg_bit_write(spi_t* spi,uint16_t address,uint8_t mask,uint8_t pos,uint8_t new_value);


#endif
