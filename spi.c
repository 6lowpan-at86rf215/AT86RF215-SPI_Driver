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
#include <linux/spi/spidev.h>
#include "spi.h"


struct spi_t at86rf215_dev={
	.name="/dev/spidev1.0",
	.mode=0,
	.bits=8,
	.speed=25000000,
	.delay=0,
	.fd=-1
};
static void pabort(const char *s)
{
	perror(s);
	if(at86rf215_dev.fd>0)
		close(at86rf215_dev.fd);
	abort();
}
void spi_init(){
	int fd = open(at86rf215_dev.name, O_RDWR);
	at86rf215_dev.fd=fd;
	if (fd < 0)
		pabort("can't open device");
	int ret = ioctl(fd, SPI_IOC_WR_MODE, &at86rf215_dev.mode);
	if (ret == -1)
		pabort("can't set spi mode");
	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &at86rf215_dev.bits);
	if (ret == -1)
		pabort("can't set bits per word");
	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &at86rf215_dev.speed);
	if (ret == -1)
		pabort("can't set max speed hz");
}

/*
* send a packet by spi and then receive a ack messaage from spi bus
*/
void spi_transfer(uint8_t*tx,uint32_t tx_len,uint8_t*rx,uint32_t rx_len)
{
	struct spi_ioc_transfer tr[2] = {
		{
			.tx_buf = (unsigned long)tx,
			.len = tx_len,
			.delay_usecs =at86rf215_dev.delay,
			.speed_hz =at86rf215_dev.speed,
			.bits_per_word =at86rf215_dev.bits,
		},
		{
			.rx_buf = (unsigned long)rx,
			.len = rx_len,
			.delay_usecs =at86rf215_dev.delay,
			.speed_hz =at86rf215_dev.speed,
			.bits_per_word =at86rf215_dev.bits,
		}
	};

	int ret = ioctl(at86rf215_dev.fd, SPI_IOC_MESSAGE(sizeof(tr)/sizeof(*tr)), &tr);
	if (ret < 1)
		pabort("can't send spi message");
}
