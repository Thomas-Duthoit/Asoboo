TARGET = game
SRC = $(TARGET).c

# Toolchain ARM
CC = arm-none-eabi-gcc
LD = arm-none-eabi-ld
OBJCOPY = arm-none-eabi-objcopy

# Flags
# -T linker.ld: use custom linker script to ensure _start is first in .text
# --gc-sections: remove unused sections
# --undefined=_start: force linker to keep _start symbol
CFLAGS = -mcpu=cortex-m0plus -mthumb -nostdlib -ffreestanding -Wall -Os
LDFLAGS = -T linker.ld --gc-sections --undefined=_start

all: $(TARGET).bin

$(TARGET).o: $(SRC) os_api.h
	$(CC) $(CFLAGS) -c $(SRC) -o $@

$(TARGET).elf: $(TARGET).o
	$(LD) $(LDFLAGS) $< -o $@ --entry=_start

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@
	@echo "Binary size: $$(wc -c < $@) bytes"

clean:
	rm -f *.o *.elf *.bin

.PHONY: all clean