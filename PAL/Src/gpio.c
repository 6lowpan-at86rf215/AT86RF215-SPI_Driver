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
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <string.h>
#include <errno.h>
#include <poll.h>
#include "gpio.h"

#define SYSFS_GPIO_DIR	"/gpio"
#define MAX_BUF 64


const char* gpio_edge_name[]={"none","both","rising","falling"};

int gpio_set_dir(unsigned int pin, unsigned int out_flag);
int gpio_set_edge(unsigned int pin, const char *edge);
int gpio_fd_open(unsigned int pin);
int gpio_fd_close(int fd);


int gpio_init(gpio_t* gpio){
	/*
	char temp_buf[50];
	sprintf(temp_buf,"echo %s > %s/direction",gpio_direction_name[gpio->direction],gpio->name);
	system(temp_buf); //set the gpio as input mode
	sprintf(temp_buf,"echo %s > %s/edge",gpio_edge_name[gpio->edge],gpio->name);
	system(temp_buf); // set the gpio interrupt mode
	sprintf(temp_buf,"%s/value",gpio->name);
	gpio->fd=open(temp_buf,O_RDONLY); 
	if(gpio->fd<0){
		perror("can't open gpio device");
		return -1;
	}
	*/
	gpio_set_dir(gpio->pin, gpio->direction);
	gpio_set_edge(gpio->pin, gpio_edge_name[gpio->edge]);
	gpio->fd=gpio_fd_open(gpio->pin);
	if(gpio->fd<0){
		perror("can't open gpio device");
		return -1;
	}

	return 0;
}

int set_gpio_value(gpio_t* gpio,gpio_value_t io_value){
	if(gpio->direction=OUT){
		char temp_buf[50];
		//printf("set gpio %d \n",io_value);
		sprintf(temp_buf,"echo %d > " SYSFS_GPIO_DIR "/pin%d/value",io_value,gpio->pin);
		system(temp_buf);
		gpio->value=io_value;
		return 0;
	}
	else
		return -1;
}

gpio_value_t get_gpio_value(gpio_t* gpio){
	uint8_t value;
	if(gpio->direction==IN){
		int ret = lseek(gpio->fd,0,SEEK_SET); 
		if( ret == -1 ){
			perror("can not uselseek");
			return -1;
		}
		ret = read(gpio->fd,&value,1); 
		if(ret==-1){
			perror("can not write");
			return -1;
		}
		value-='0';
	}
	else
		value=gpio->value;
	gpio->value=value;
	return gpio->value;
}

/****************************************************************
 * gpio_set_direction
 0 in
 1 out
 ****************************************************************/
int gpio_set_dir(unsigned int pin, unsigned int out_flag)
{
	int fd, len;
	char buf[MAX_BUF];
 
	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR  "/pin%d/direction", pin);
 
	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("gpio/direction");
		return fd;
	}
 
	if (out_flag)
		write(fd, "out", 4);
	else
		write(fd, "in", 3);
 
	close(fd);
	return 0;
}

/****************************************************************
 * gpio_set_value
 0 low
 1 high
 ****************************************************************/
 /*
int gpio_set_value(unsigned int pin, unsigned int value)
{
	int fd, len;
	char buf[MAX_BUF];
 
	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/pin%d/value", pin);
 
	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("gpio/set-value");
		return fd;
	}
 
	if (value)
		write(fd, "1", 2);
	else
		write(fd, "0", 2);
 
	close(fd);
	return 0;
}
*/
/****************************************************************
 * gpio_get_value
 ****************************************************************/
 /*
int gpio_get_value(unsigned int pin, unsigned int *value)
{
	int fd, len;
	char buf[MAX_BUF];
	char ch;

	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/pin%d/value", pin);
 
	fd = open(buf, O_RDONLY);
	if (fd < 0) {
		perror("gpio/get-value");
		return fd;
	}
 
	read(fd, &ch, 1);

	if (ch != '0') {
		*value = 1;
	} else {
		*value = 0;
	}
 
	close(fd);
	return 0;
}
*/

/****************************************************************
 * gpio_set_edge
 ****************************************************************/

int gpio_set_edge(unsigned int pin, const char *edge)
{
	int fd, len;
	char buf[MAX_BUF];

	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/pin%d/edge", pin);
 
	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("gpio/set-edge");
		return fd;
	}
 
	write(fd, edge, strlen(edge) + 1); 
	close(fd);
	return 0;
}

/****************************************************************
 * gpio_fd_open
 ****************************************************************/

int gpio_fd_open(unsigned int pin)
{
	int fd, len;
	char buf[MAX_BUF];

	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/pin%d/value", pin);
 
	fd = open(buf, O_RDONLY | O_NONBLOCK );
	if (fd < 0) {
		perror("gpio/fd_open");
	}
	return fd;
}

/****************************************************************
 * gpio_fd_close
 ****************************************************************/

int gpio_fd_close(int fd)
{
	return close(fd);
}

