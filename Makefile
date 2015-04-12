
CFLAGS = -O2 -static -Wall


all: init

init: virtramfs.c
	$(CC) $(CFLAGS) -o $@ $^
