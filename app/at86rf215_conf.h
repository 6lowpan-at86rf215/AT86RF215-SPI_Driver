#ifndef _AT86RF215_CONF
#define _AT86RF215_CONF
#include "spi.h"
#include "gpio.h"
struct At86rf215_Dev_t{
	struct spi_t at86rf215_spi;
	struct gpio_t at86rf215_gpio;
};

#endif