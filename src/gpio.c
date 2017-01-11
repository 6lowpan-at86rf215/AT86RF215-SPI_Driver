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

extern struct At86rf215_Dev_t at86rf215_dev;

const char* gpio_edge_name[]={"null","both","rising","falling"};

int gpio_init(){
	char temp_buf[50];
	sprintf(temp_buf,"echo in > %s/direction",at86rf215_dev.at86rf215_gpio.name);
	system(temp_buf); //set the gpio as input mode
	sprintf(temp_buf,"echo %s > %s/direction",gpio_edge_name[at86rf215_dev.at86rf215_gpio.edge],at86rf215_dev.at86rf215_gpio.name);
	system(temp_buf); // set the gpio interrupt mode
	sprintf(temp_buf,"%s/value",at86rf215_dev.at86rf215_gpio.name);
	at86rf215_dev.at86rf215_gpio.fd=open(temp_buf,O_RDONLY); 
	if(at86rf215_dev.at86rf215_gpio.fd<0){
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
