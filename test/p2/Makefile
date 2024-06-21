CC=aarch64-linux-gnu-gcc-10
CFLAGS=-Wall $(INCLUDE_FLAGS) -ffreestanding -g
LD=aarch64-linux-gnu-ld
LD_SCRIPT=linker.ld
OBJCOPY=aarch64-linux-gnu-objcopy

ROOTFS=../../rootfs

.PHONY: clean

all: p2.img

p2.img: p2.elf
	$(OBJCOPY) -O binary p2.elf p2.img
	cp p2.img $(ROOTFS)/p2.img

p2.elf:
	$(CC) $(CFLAGS) -o syscall.o -c ../shared/syscall.S
	$(CC) $(CFLAGS) -o start.o -c start.S
	$(CC) $(CFLAGS) -o main.o -c main.c
	$(LD) -T $(LD_SCRIPT) -o p2.elf start.o syscall.o main.o

clean:
	@rm -rf start.o p2.img p2.elf
