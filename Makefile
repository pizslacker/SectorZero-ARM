# Toolchain definition
CROSS_COMPILE ?= aarch64-linux-gnu-
CC      = $(CROSS_COMPILE)gcc
LD      = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy

# Compiler and Linker flags
CFLAGS  = -Wall -O2 -ffreestanding -nostartfiles
LDFLAGS = -T linker.ld -nostdlib

# Project files
OBJS    = boot.o kernel.o
ELF     = SectorZero.elf
IMG     = kernel8.img

# Default target
.PHONY: all clean

all: $(IMG)

# Pattern rules for compiling C and Assembly
%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Link the object files into an ELF executable
$(ELF): $(OBJS)
	# boot.o is listed first in $(OBJS) to ensure it is placed correctly
	$(LD) $(LDFLAGS) $(OBJS) -o $@

# Extract the raw binary payload
$(IMG): $(ELF)
	$(OBJCOPY) -O binary $< $@

# Clean build artifacts
clean:
	rm -f $(OBJS) $(ELF) $(IMG)