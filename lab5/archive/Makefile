LD = aarch64-linux-gnu-ld
LDFLAGS = -T linker.ld
CXX = aarch64-linux-gnu-gcc
OBJ = $(wildcard *.o)
SRC = $(wildcard *.s)
CXXFLAGS = -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles
OBJCOPY = aarch64-linux-gnu-objcopy
OBJCOPYFLAGS = -O binary

all:
	make user_prog
	cd rootfs&&find . | cpio -o -H newc > ../initramfs.cpio
	cd .. | cp initramfs.cpio ../

user_prog:
	$(CXX) $(CXXFLAGS) -c $(SRC)
	$(LD) $(LDFLAGS) -o tmp.elf $(OBJ)
	$(OBJCOPY) $(OBJCOPYFLAGS) tmp.elf $@
	rm tmp.elf
	mv $@ ./rootfs/$@
	rm *.o
clean:
	rm *.cpio