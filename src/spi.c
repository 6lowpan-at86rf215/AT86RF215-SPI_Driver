/*
 * AT86RF215 test code with spi driver
 *
 * Copyright (c) 2016  Cisco, Inc.
 * Copyright (c) 2016  <binyao@cisco.com>
 *
 */
#include <stdint.h>
#include <unistd.h>
/*
 * AT86RF215 test code with spi driver
 *
 * Copyright (c) 2017  Cisco, Inc.
 * Copyright (c) 2017  <binyao@cisco.com>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include "spi.h"
#include "at86rf215_conf.h"

//extern struct At86rf215_Dev_t at86rf215_dev;
static inline uint16_t set_spi_address(uint16_t address){
	uint16_t res=(address>>8)&0xff;
	res|=(address&0xff)<<8;
	return res;
}

int spi_init(struct spi_t* spi){
	spi->fd = open(spi->name, O_RDWR);
	if (spi->fd < 0){
		perror("can't open spi device");
		return -1;
	}
	int ret = ioctl(spi->fd, SPI_IOC_WR_MODE, &(spi->mode));
	if (ret == -1){
		perror("can't set spi mode");
		return -1;
	}
	ret = ioctl(spi->fd, SPI_IOC_WR_BITS_PER_WORD, &(spi->bits));
	if (ret == -1){
		perror("can't set bits per word");
		return -1;
	}
	ret = ioctl(spi->fd, SPI_IOC_WR_MAX_SPEED_HZ, &(spi->speed));
	if (ret == -1){
		perror("can't set max speed hz");
		return -1;
	}
	return 0;
}

int spi_write(struct spi_t* spi,struct spi_data_t* data){
	uint16_t spi_address=set_spi_address(data->address);//改变高低位
	spi_address|=(1<<7);// set write mode
	struct spi_ioc_transfer tr[2] = {
		{
			.tx_buf = (unsigned long)&spi_address,
			.len = sizeof(data->address)/sizeof(uint8_t),
			.delay_usecs =spi->delay,
			.speed_hz =spi->speed,
			.bits_per_word =spi->bits,
		},
		{
			.tx_buf = (unsigned long)data->data,
			.len = data->len,
			.delay_usecs =spi->delay,
			.speed_hz =spi->speed,
			.bits_per_word =spi->bits,
		}
	};

	int ret = ioctl(spi->fd, SPI_IOC_MESSAGE(sizeof(tr)/sizeof(*tr)), &tr);
	if (ret < 1){
		perror("can't send spi message");
		return -1;
	}
	return data->len;
}

int spi_read(struct spi_t* spi,struct spi_data_t *data){
	uint16_t spi_address=set_spi_address(data->address);
	struct spi_ioc_transfer tr[2] = {
		{
			.tx_buf = (unsigned long)&spi_address,
			.len = 2,
			.delay_usecs =spi->delay,
			.speed_hz =spi->speed,
			.bits_per_word =spi->bits,
		},
		{
			.rx_buf = (unsigned long)data->data,
			.len = data->len,
			.delay_usecs =spi->delay,
			.speed_hz =spi->speed,
			.bits_per_word =spi->bits,
		}
	};

	int ret = ioctl(spi->fd, SPI_IOC_MESSAGE(sizeof(tr)/sizeof(*tr)), &tr);
	if (ret < 1){
		perror("can't send spi message");
		return -1;
	}
	return data->len;

}

