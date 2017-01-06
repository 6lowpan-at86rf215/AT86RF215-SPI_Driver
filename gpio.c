#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <poll.h>
static void pabort(const char *s)
{
	perror(s);
	//if(at86rf215_dev.fd>0)
	//	close(at86rf215_dev.fd);
	abort();
}
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
}
