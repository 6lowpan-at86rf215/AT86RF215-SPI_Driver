#ifndef _GPIO_H
#define _GPIO_H
enum gpio_edge{
	none,both,rising,falling
};
enum gpio_direction{
	in,out
};
enum gpio_value{
	low,high
};

struct gpio_t{
	char *name;
	int fd;
	enum gpio_direction direction;
	enum gpio_edge edge;
	enum gpio_value value;
};
int gpio_init(struct gpio_t* gpio);
int set_gpio_value(struct gpio_t* gpio,enum gpio_value io_value);
enum gpio_value  get_gpio_value(struct gpio_t* gpio);


#endif
