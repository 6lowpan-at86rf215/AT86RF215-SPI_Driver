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

struct spi_data_t{
	uint16_t address;
	uint8_t* data;
	uint32_t len;
};

int spi_init(void);
int spi_write(struct spi_data_t* data);
int spi_read(struct spi_data_t *data);

#endif
