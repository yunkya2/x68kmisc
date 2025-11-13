CROSS = m68k-xelf-
CC = $(CROSS)gcc
AS = $(CROSS)gcc
LD = $(CROSS)gcc
AR = $(CROSS)ar
RANLIB = $(CROSS)ranlib

TARGET = iobuf

all: $(TARGET).x

%.x: %.o
	$(LD) -o $@ $^ $(LDFLAGS) -s -specs=nano.specs

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	-rm -f *.o *.x *.elf
