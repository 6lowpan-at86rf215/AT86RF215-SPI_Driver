target=at86rf215-dev
CC:=arm-linux-gnueabihf-gcc
CUR_DIR := .
PATH_TARGET := $(CUR_DIR)/bin
PATH_OBJ := $(CUR_DIR)/obj
PATH_APP := $(CUR_DIR)/app
PATH_AT86RF215 := $(CUR_DIR)/at86rf215
PATH_SRC := $(CUR_DIR)/src
PATH_INC := $(CUR_DIR)/include
INCLUDES = -I $(PATH_INC)
INCLUDES += -I $(CUR_DIR)/app
INCLUDES += -I $(PATH_AT86RF215)/include
CFLAGS :=-DSUPPORT_LEGACY_OQPSK -DSUPPORT_OQPSK -DSUPPORT_OFDM -DSUPPORT_FSK
CFLAGS += -DNUM_TRX=2			# define NUM_TRX=2
CFLAGS += -DRF215v3				# define RF215v3
target := $(PATH_TARGET)/$(target)
objects := $(PATH_OBJ)/main.o	\
	$(PATH_OBJ)/spi.o	\
	$(PATH_OBJ)/gpio.o	\
	$(PATH_OBJ)/tal_init.o	\
	$(PATH_OBJ)/tal.o	\
	$(PATH_OBJ)/tal_irq_handler.o
$(target):$(objects)
	$(CC)  -o $@ $^
$(PATH_OBJ)/main.o:$(PATH_APP)/main.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<
$(PATH_OBJ)/spi.o:$(PATH_SRC)/spi.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<
$(PATH_OBJ)/gpio.o:$(PATH_SRC)/gpio.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<
$(PATH_OBJ)/tal_init.o:$(PATH_AT86RF215)/src/tal_init.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<
$(PATH_OBJ)/tal.o:$(PATH_AT86RF215)/src/tal.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<
$(PATH_OBJ)/tal_irq_handler.o:$(PATH_AT86RF215)/src/tal_irq_handler.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<
.PHONY:clean
clean:
	rm -rf $(target) $(objects)
