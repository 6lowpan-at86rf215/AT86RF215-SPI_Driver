target=at86rf215-dev
CC:=arm-linux-gnueabihf-gcc
CUR_DIR := .
PATH_TARGET := $(CUR_DIR)/bin
PATH_OBJ := $(CUR_DIR)/obj
PATH_APP := $(CUR_DIR)/app
PATH_SRC := $(CUR_DIR)/src
PATH_INC := $(CUR_DIR)/inc
INCLUDES := -I $(PATH_INC)
target := $(PATH_TARGET)/$(target)
objects := $(PATH_OBJ)/main.o \
	$(PATH_OBJ)/spi.o
$(target):$(objects)
	$(CC) -o $@ $^
$(PATH_OBJ)/main.o:$(PATH_APP)/main.c
	$(CC) $(INCLUDES) -c -o $@ $<
$(PATH_OBJ)/spi.o:$(PATH_SRC)/spi.c
	$(CC) $(INCLUDES) -c -o $@ $<
.PHONY:clean
clean:
	rm -rf $(target) $(objects)
