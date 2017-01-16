/*
 * AT86RF215 test code with spi driver
 *
 * Copyright (c) 2017  Cisco, Inc.
 * Copyright (c) 2017  <binyao@cisco.com>
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
struct spi_t at86rf215_spi={
	.name="/dev/spidev1.0",
	.mode=0,
	.bits=8,
	.speed=25000000,
	.delay=0,
	.fd=-1

};

struct gpio_t at86rf215_gpio_irq={
	.name="/gpio/pin25",
	.fd=-1,
	.direction=in,
	.edge=both,
	.value=low
};

struct gpio_t at86rf215_gpio_rest={
	.name="/gpio/pin24",
	.fd=-1,
	.direction=out,
	.edge=none,
	.value=low
};

struct At86rf215_Dev_t at86rf215_dev={
	.spi=&at86rf215_spi,
	.gpio_irq=&at86rf215_gpio_irq,
	.gpio_rest=&at86rf215_gpio_rest
};

static void clean(void);

int main(int argc, char *argv[]){
	int i,ret;
	uint16_t address=0x0005;
	uint8_t cmd=0x03;
	uint8_t rx[1];
	atexit(clean);
	if(-1==spi_init(at86rf215_dev.spi)){
		return -1;
	}
	if(-1==gpio_init(at86rf215_dev.gpio_irq)){
		return -1;
	}
	if(-1==gpio_init(at86rf215_dev.gpio_rest)){
		return -1;
	}
	/*
	set_gpio_value(at86rf215_dev.gpio_rest,low);
	printf("read value=%d\n",get_gpio_value(at86rf215_dev.gpio_rest));
	set_gpio_value(at86rf215_dev.gpio_rest,high);
	printf("read value=%d\n",get_gpio_value(at86rf215_dev.gpio_rest));
	*/
	struct spi_data_t message={
		.address=address,
		.data=rx,
		.len=sizeof(rx)/sizeof(uint8_t)
	};

	message.data=rx;
	message.len=sizeof(rx)/sizeof(uint8_t);
	ret=spi_read(at86rf215_dev.spi,&message);
	printf("send address: %.4X\n",message.address);
	printf("recv mesage:");
	for(i=0;i<message.len;i++)
		printf("%.2X ", message.data[i]);
	printf("\n");

	printf("write 0x03\n");
	message.data=&cmd;
	message.len=sizeof(cmd)/sizeof(uint8_t);
	ret=spi_write(at86rf215_dev.spi,&message);

	
	message.data=rx;
	message.len=sizeof(rx)/sizeof(uint8_t);
	ret=spi_read(at86rf215_dev.spi,&message);
	printf("send address: %.4X\n",message.address);
	printf("recv mesage:");
	for(i=0;i<message.len;i++)
		printf("%.2X ", message.data[i]);
	printf("\n");

	printf("reset at86rf215\n");
	printf("status=%d\n",tal_init(&at86rf215_dev));
	
	message.data=rx;
	message.len=sizeof(rx)/sizeof(uint8_t);
	ret=spi_read(at86rf215_dev.spi,&message);
	printf("send address: %.4X\n",message.address);
	printf("recv mesage:");
	for(i=0;i<message.len;i++)
		printf("%.2X ", message.data[i]);
	printf("\n");

	
}

static void clean(void){
	close(at86rf215_dev.spi->fd);
	close(at86rf215_dev.gpio_irq->fd);
}

