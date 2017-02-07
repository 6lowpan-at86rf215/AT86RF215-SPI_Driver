############################################################################################
# Makefile for Chat
# $Id: Makefile 37628 2015-07-14 14:36:46Z uwalter $
############################################################################################
TARGET=at86rf215-dev-1235_1234
# Build specific properties
_TAL_TYPE = AT86RF215
_HIGHEST_STACK_LAYER = TAL
_PAL_GENERIC_TYPE = IMX6SX
_PAL_TYPE = IMX6SX
_MCU_DEVICE = IMX6SX
_BOARD_TYPE = UDOO-NEO
_SHORTENUM=

## General Flags
CC=arm-linux-gnueabihf-gcc
# Path variables
## Path to main project directory
PATH_ROOT = .
TARGET_DIR = ./bin
PATH_APP = ./Applications
PATH_GLOB_INC = $(PATH_ROOT)/Include
PATH_TAL = $(PATH_ROOT)/TAL
PATH_TAL_CB = $(PATH_ROOT)/TAL/Src
PATH_PAL = $(PATH_ROOT)/PAL
PATH_RES = $(PATH_ROOT)/Resources
PATH_RES = $(PATH_ROOT)/Resources


# Use debug USB via UART1
#CFLAGS += -Wall -Werror -g3 -Wundef -std=c99 -Os -fshort-enums -ffunction-sections
CFLAGS += -std=gnu99
CFLAGS += -DSIO_HUB -DENABLE_UART1 -DBAUD_RATE=115200 -DSUPPORT_LEGACY_OQPSK -DSUPPORT_OQPSK -DSUPPORT_OFDM -DSUPPORT_FSK
CFLAGS += -DDEBUG=0 -DRF215v3
CFLAGS += -DENABLE_LARGE_BUFFER
CFLAGS += -DTAL_TYPE=$(_TAL_TYPE)
CFLAGS += -DHIGHEST_STACK_LAYER=$(_HIGHEST_STACK_LAYER)
CFLAGS += -DSHORTENUM=$(_SHORTENUM)
CFLAGS += -DPAL_GENERIC_TYPE=$(_PAL_GENERIC_TYPE)
CFLAGS += -DPAL_TYPE=$(_PAL_TYPE)
#open the gdb debug mode
CFLAGS += -g

## Include directories for application
INCLUDES += -I $(PATH_APP)/Inc
## Include directories for general includes
INCLUDES += -I $(PATH_GLOB_INC)
## Include directories for resources
INCLUDES += -I $(PATH_RES)/Buffer_Management/Inc/
INCLUDES += -I $(PATH_RES)/Queue_Management/Inc/
## Include directories for TAL
INCLUDES += -I $(PATH_TAL)/Inc/
INCLUDES += -I $(PATH_TAL)/$(_TAL_TYPE)/Inc/
## Include directories for PAL
INCLUDES += -I $(PATH_PAL)/Inc/


OBJECTS = \
	$(TARGET_DIR)/main.o\
	$(TARGET_DIR)/spi.o\
	$(TARGET_DIR)/gpio.o\
	$(TARGET_DIR)/pal.o\
	$(TARGET_DIR)/bmm.o\
	$(TARGET_DIR)/qmm.o\
	$(TARGET_DIR)/tal.o\
	$(TARGET_DIR)/tal_auto_rx.o\
	$(TARGET_DIR)/tal_auto_tx.o\
	$(TARGET_DIR)/tal_fe.o\
	$(TARGET_DIR)/tal_ed.o\
	$(TARGET_DIR)/tal_pib.o\
	$(TARGET_DIR)/tal_init.o\
	$(TARGET_DIR)/tal_irq_handler.o\
	$(TARGET_DIR)/tal_pwr_mgmt.o\
	$(TARGET_DIR)/tal_rx_enable.o \
	$(TARGET_DIR)/tal_auto_ack.o \
	$(TARGET_DIR)/tal_auto_csma.o \
	$(TARGET_DIR)/tal_4g_utils.o \
	$(TARGET_DIR)/tal_phy_cfg.o \
	$(TARGET_DIR)/tal_ftn.o \
	$(TARGET_DIR)/tal_rand.o \
	$(TARGET_DIR)/pal_trx_spi_block_mode.o	\
	$(TARGET_DIR)/phy_conf.o	\
	$(TARGET_DIR)/chat.o

$(TARGET_DIR)/$(TARGET):$(OBJECTS)
	$(CC)  -o $@ $^ -lrt
$(TARGET_DIR)/bmm.o: $(PATH_RES)/Buffer_Management/Src/bmm.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/qmm.o: $(PATH_RES)/Queue_Management/Src/qmm.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/tal.o: $(PATH_TAL)/$(_TAL_TYPE)/Src/tal.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/tal_auto_rx.o: $(PATH_TAL)/$(_TAL_TYPE)/Src/tal_auto_rx.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/tal_auto_tx.o: $(PATH_TAL)/$(_TAL_TYPE)/Src/tal_auto_tx.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/tal_ed.o: $(PATH_TAL)/$(_TAL_TYPE)/Src/tal_ed.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/tal_fe.o: $(PATH_TAL)/$(_TAL_TYPE)/Src/tal_fe.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/tal_pib.o: $(PATH_TAL)/$(_TAL_TYPE)/Src/tal_pib.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/tal_init.o: $(PATH_TAL)/$(_TAL_TYPE)/Src/tal_init.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/tal_irq_handler.o: $(PATH_TAL)/$(_TAL_TYPE)/Src/tal_irq_handler.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/tal_pwr_mgmt.o: $(PATH_TAL)/$(_TAL_TYPE)/Src/tal_pwr_mgmt.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/tal_rx_enable.o: $(PATH_TAL)/$(_TAL_TYPE)/Src/tal_rx_enable.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/tal_auto_ack.o: $(PATH_TAL)/$(_TAL_TYPE)/Src/tal_auto_ack.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/tal_auto_csma.o: $(PATH_TAL)/$(_TAL_TYPE)/Src/tal_auto_csma.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/tal_4g_utils.o: $(PATH_TAL)/$(_TAL_TYPE)/Src/tal_4g_utils.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/tal_phy_cfg.o: $(PATH_TAL)/$(_TAL_TYPE)/Src/tal_phy_cfg.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/tal_ftn.o: $(PATH_TAL)/$(_TAL_TYPE)/Src/tal_ftn.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/tal_rand.o: $(PATH_TAL)/$(_TAL_TYPE)/Src/tal_rand.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/pal_trx_spi_block_mode.o: $(PATH_PAL)/Src/pal_trx_spi_block_mode.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/spi.o: $(PATH_PAL)/Src/spi.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/gpio.o: $(PATH_PAL)/Src/gpio.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/pal.o: $(PATH_PAL)/Src/Pal.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/main.o: $(PATH_APP)/Src/main.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/phy_conf.o: $(PATH_APP)/Src/phy_conf.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/chat.o: $(PATH_APP)/Src/chat.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
all:Pal Tal Main
.PHONY:Main
Main:
	make $(TARGET_DIR)/main.o
.PHONY:Pal
Pal:
	make $(TARGET_DIR)/spi.o
	make $(TARGET_DIR)/gpio.o
	make $(TARGET_DIR)/pal.o
	make $(TARGET_DIR)/pal_trx_spi_block_mode.o
.PHONY:Tal
Tal:
	make $(TARGET_DIR)/bmm.o
	make $(TARGET_DIR)/qmm.o
	make $(TARGET_DIR)/tal.o
	make $(TARGET_DIR)/tal_auto_rx.o
	make $(TARGET_DIR)/tal_auto_tx.o
	make $(TARGET_DIR)/tal_ed.o
	make $(TARGET_DIR)/tal_fe.o
	make $(TARGET_DIR)/tal_pib.o
	make $(TARGET_DIR)/tal_init.o
	make $(TARGET_DIR)/tal_irq_handler.o
	make $(TARGET_DIR)/tal_pwr_mgmt.o
	make $(TARGET_DIR)/tal_rx_enable.o
	make $(TARGET_DIR)/tal_4g_utils.o
	make $(TARGET_DIR)/tal_phy_cfg.o
	make $(TARGET_DIR)/tal_ftn.o
	make $(TARGET_DIR)/tal_rand.o
	make $(TARGET_DIR)/tal_auto_ack.o
	make $(TARGET_DIR)/tal_auto_csma.o
.PHONY:Gpio
Gpio:
	$(CC) -c $(CFLAGS) $(INCLUDES) -o Gpio-int-test.o Gpio-int-test.c
	$(CC) -o gpio-test Gpio-int-test.o
.PHONY:clean
clean:
	rm -rf $(TARGET) $(OBJECTS)