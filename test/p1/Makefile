CC=aarch64-linux-gnu-gcc-10
CFLAGS=-Wall $(INCLUDE_FLAGS) -ffreestanding -g
LD=aarch64-linux-gnu-ld
LD_SCRIPT=linker.ld
OBJCOPY=aarch64-linux-gnu-objcopy

ROOTFS=../../rootfs

.PHONY: clean

all: p1.img

p1.img: p1.elf
	$(OBJCOPY) -O binary p1.elf p1.img
	cp p1.img $(ROOTFS)/p1.img

p1.elf:
	$(CC) $(CFLAGS) -o start.o -c start.S
	$(LD) -T $(LD_SCRIPT) -o p1.elf start.o

clean:
	@rm -rf start.o p1.img p1.elf
