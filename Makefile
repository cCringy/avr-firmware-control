MCU = atmega328p
F_CPU = 16000000UL

PROGRAMMER_ARGS = -P COM9
BUILD_DIR = build

# Arduino IDE bringt avr-gcc/avrdude schon mit, sie liegen nur nicht im PATH
GCC_DIR = /c/Users/rfick/AppData/Local/Arduino15/packages/arduino/tools/avr-gcc/7.3.0-atmel3.6.1-arduino7/bin
AVRDUDE_DIR = /c/Users/rfick/AppData/Local/Arduino15/packages/arduino/tools/avrdude/6.3.0-arduino17

CC = "$(GCC_DIR)/avr-gcc.exe"
OBJCOPY = "$(GCC_DIR)/avr-objcopy.exe"
AVRDUDE = "$(AVRDUDE_DIR)/bin/avrdude.exe" -C "$(AVRDUDE_DIR)/etc/avrdude.conf"

UART_FRAMESIZE ?= 8

CFLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -DUART_FRAMESIZE=$(UART_FRAMESIZE)
CFLAGS += -Iapp -Ibsp -Ihal -Idsp -Iutil

# alle .c Dateien aus diesen Ordnern einsammeln
SRCS = $(wildcard app/*.c bsp/*.c hal/*.c dsp/*.c util/*.c)
# jede z.B. hal/gpio.c wird zu build/hal/gpio.o
OBJS = $(patsubst %.c, $(BUILD_DIR)/%.o, $(SRCS))

$(BUILD_DIR)/%.o: %.c
	mkdir -p $(dir $@) && $(CC) $(CFLAGS) -c $< -o $@

main.elf: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -lm -o main.elf

main.hex: main.elf
	$(OBJCOPY) -O ihex -R .eeprom main.elf main.hex

flash: main.hex
	$(AVRDUDE) -c arduino -p $(MCU) $(PROGRAMMER_ARGS) -D -V -U flash:w:$<

.PHONY: clean all flash

all : main.hex

clean :
	rm -rf $(BUILD_DIR) && rm -f main.elf main.hex
