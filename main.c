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
#include "spi.h"

extern struct spi_t at86rf215_dev;
void clean(void);

int main(int argc, char *argv[]){
	int i;
	uint8_t tx[]={0x00,0x0D};
	uint8_t rx[2];
	atexit(clean);
	spi_init();
	spi_transfer(tx,2,rx,2);
	printf("send address: %.2X %.2X \n",tx[0],tx[1]);
	printf("recv mesage:\n");
	for(i=0;i<2;i++)
		printf("%.2X ", rx[i]);
	printf("\n");
	
}

void clean(void){
	close(at86rf215_dev.fd);
}

