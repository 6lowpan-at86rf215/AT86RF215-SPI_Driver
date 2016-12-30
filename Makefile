objects:=main.o spi.o
target:=ATRF215-Driver
CC:=arm-linux-gnueabihf-gcc-4.8
$(target):$(objects)
	$(CC) -o $@ $^
.PHONY:clean
clean:
	rm -rf $(target) $(objects)
