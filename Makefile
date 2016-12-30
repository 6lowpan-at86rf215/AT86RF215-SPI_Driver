objects:=main.o spi.o
target:=at86rf815-dev
CC:=arm-linux-gnueabihf-gcc-4.8
$(target):$(objects)
	$(CC) -o $@ $^
.PHONY:clean
clean:
	rm -rf $(target) $(objects)
