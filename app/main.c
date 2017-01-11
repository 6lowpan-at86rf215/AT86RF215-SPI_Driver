/*
 * AT86RF215 test code with spi driver
 *
 * Copyright (c) 2016  Cisco, Inc.
 * Copyright (c) 2016  <binyao@cisco.com>
 *
 */
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include "at86rf215_conf.h"

struct At86rf215_Dev_t at86rf215_dev={
	.at86rf215_spi={
		.name="/dev/spidev1.0",
		.mode=0,
		.bits=8,
		.speed=25000000,
		.delay=0,
		.fd=-1
	},
	.at86rf215_gpio={
		.name="/gpio/pin25",
		.fd=-1,
		.edge=both
	}
};

static void clean(void);

int main(int argc, char *argv[]){
	int i,ret;
	uint16_t address=0x0005;
	uint8_t cmd=0x03;
	uint8_t rx[1];
	atexit(clean);
	if(-1==spi_init()){
		return -1;
	}
	if(-1==gpio_init()){
		return -1;
	}
	struct spi_data_t message={
		.address=address,
		.data=rx,
		.len=sizeof(rx)/sizeof(uint8_t)
	};
	ret=spi_read(&message);
	printf("send address: %.4X\n",message.address);
	printf("recv mesage:\n");
	for(i=0;i<message.len;i++)
		printf("%.2X ", message.data[i]);
	printf("\n");
	message.data=&cmd;
	message.len=sizeof(cmd)/sizeof(uint8_t);
	ret=spi_write(&message);
	message.data=rx;
	message.len=sizeof(rx)/sizeof(uint8_t);
	ret=spi_read(&message);
	printf("send address: %.4X\n",message.address);
	printf("recv mesage:\n");
	for(i=0;i<message.len;i++)
		printf("%.2X ", message.data[i]);
	printf("\n");

	
}

static void clean(void){
	close(at86rf215_dev.at86rf215_spi.fd);
}

