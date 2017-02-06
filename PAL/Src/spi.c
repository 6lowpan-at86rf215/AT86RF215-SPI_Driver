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
#include "pal.h"

static inline uint16_t set_spi_address(uint16_t address){
	uint16_t res=(address>>8)&0xff;
	res|=(address&0xff)<<8;
	return res;
}

int spi_init(spi_t* spi){
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

int spi_write(spi_t* spi,spi_data_t* data){
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

int spi_read(spi_t* spi,spi_data_t *data){
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

uint8_t spi_reg_read(spi_t* spi,uint16_t address){
	uint8_t rx[1];
	spi_data_t message={
		.address=address,
		.data=rx,
		.len=sizeof(rx)/sizeof(uint8_t)
	};
	if(-1==spi_read(spi,&message)){
		return -1;
	}
	return message.data[0];
}

int spi_reg_write(spi_t* spi,uint16_t address,uint8_t value){
	spi_data_t message={
		.address=address,
		.data=&value,
		.len=sizeof(value)/sizeof(uint8_t)
	};
	int ret=spi_write(spi,&message);
	if(-1==ret){
		return -1;
	}
	return ret;
}

uint8_t spi_reg_bit_read(spi_t* spi,uint16_t address,uint8_t mask,uint8_t pos){
	uint8_t ret=spi_reg_read(spi,address);
	ret&=mask;
	ret>>pos;
	return ret;
}

uint8_t spi_reg_bit_write(spi_t* spi,uint16_t address,uint8_t mask,uint8_t pos,uint8_t new_value){
	uint8_t current_reg_value=spi_reg_read(spi,address);
	current_reg_value&=~(mask);
	new_value<<=pos;
	new_value&=mask;
	new_value|=current_reg_value;
	return spi_reg_write(spi,address,new_value);
}



