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
#include <poll.h>
#include "gpio.h"
#include "at86rf215_conf.h"


const char* gpio_edge_name[]={"none","both","rising","falling"};
const char* gpio_direction_name[]={"in","out"};

int gpio_init(struct gpio_t* gpio){
	char temp_buf[50];
	sprintf(temp_buf,"echo %s > %s/direction",gpio_direction_name[gpio->direction],gpio->name);
	printf("temp_buf = %s\n",temp_buf);
	system(temp_buf); //set the gpio as input mode
	sprintf(temp_buf,"echo %s > %s/edge",gpio_edge_name[gpio->edge],gpio->name);
	printf("temp_buf = %s\n",temp_buf);
	system(temp_buf); // set the gpio interrupt mode
	sprintf(temp_buf,"%s/value",gpio->name);
	printf("temp_buf = %s\n",temp_buf);
	gpio->fd=open(temp_buf,O_RDONLY); 
	if(gpio->fd<0){
		perror("can't open gpio device");
		return -1;
	}
	return 0;
}
/*
int main()
{
	system("echo in > /gpio/pin25/direction"); 
	system("echo both > /gpio/pin25/edge"); 
	int gpio_fd = open("/gpio/pin25/value",O_RDONLY); 
	struct pollfd fds[1]; 
	unsigned char value;
	if(-1==gpio_fd)
		pabort("can't open device");
	fds[0].fd=gpio_fd;
	fds[0].events = POLLPRI;
	int ret = lseek(gpio_fd,0,SEEK_SET);
	if(-1==ret)
		pabort("lseek");
	ret = read(gpio_fd,&value,1); 
	if (ret == -1 ) 
		pabort("read");
	while (1) { 
		ret = poll(fds,1,-1); 
		if ( ret == -1 ) 
			pabort("poll"); 
		if ( fds[0].revents & POLLPRI ){ 
			ret = lseek(gpio_fd,0,SEEK_SET); 
			if( ret == -1 ) 
				pabort("lseek"); 
			ret = read(gpio_fd,&value,1); 
			if( ret == -1 ) 
				pabort("read"); 
			printf("value=%d\n",value);
		} 
	}  
	close( gpio_fd );
	return 0;
}*/
