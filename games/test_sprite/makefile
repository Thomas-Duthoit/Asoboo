TARGET = game
SRC = $(TARGET).c

# Toolchain ARM
CC = arm-none-eabi-gcc
LD = arm-none-eabi-ld
OBJCOPY = arm-none-eabi-objcopy

# Flags
CFLAGS = -mcpu=cortex-m0plus -mthumb -nostdlib -ffreestanding -Wall -Os
LDFLAGS = -Ttext=0x20030000 --gc-sections

all: $(TARGET).bin

$(TARGET).o: $(SRC) os_api.h
	$(CC) $(CFLAGS) -c $(SRC) -o $@

$(TARGET).elf: $(TARGET).o
	$(LD) $(LDFLAGS) $< -o $@ --entry=game_main

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@
	@echo "Binary size: $$(wc -c < $@) bytes"

clean:
	rm -f *.o *.elf *.bin

.PHONY: all clean