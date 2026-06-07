MCU = atmega328p
F_CPU = 16000000UL

PROGRAMMER_ARGS = -P COM9
BUILD_DIR = build
AVRDUDE = avrdude

#src/main.o  src/scheduler.o  build/task.o
SRCS = $(wildcard src/*.c)
#build/main.o  build/scheduler.o  build/task.o
OBJS = $(patsubst src/%.c, build/%.o, $(SRCS))



build/%.o: src/%.c
  mkdir -p $(BUILD_DIR) && avr-gcc -mmcu=$(MCU) -DF_CPU=$(F_CPU) -c $< -o $@

main.elf: $(OBJS)
	avr-gcc -mmcu=$(MCU) -DF_CPU=$(F_CPU) $(OBJS) -o main.elf

main.hex: main.elf
	avr-objcopy -O ihex -R .eeprom main.elf main.hex

flash: main.hex
	$(AVRDUDE) -c arduino -p $(MCU) $(PROGRAMMER_ARGS) -D -V -U flash:w:$<

.PHONY: clean all

all : main.hex

clean : 
	rm -rf build $$ rm main.elf main.hex


