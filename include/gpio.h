#ifndef _GPIO_H
#define _GPIO_H
enum gpio_edge{
	none,both,rising,falling
};
enum gpio_direction{
	in,out
};
struct gpio_t{
	char *name;
	int fd;
	enum gpio_direction direction;
	enum gpio_edge edge;
};
int gpio_init(struct gpio_t* gpio);

#endif
