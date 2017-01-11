#ifndef _GPIO_H
#define _GPIO_H
enum gpio_edge{
	null,both,rising,falling
};

struct gpio_t{
	char *name;
	int fd;
	enum gpio_edge edge;
};
int gpio_init();

#endif
