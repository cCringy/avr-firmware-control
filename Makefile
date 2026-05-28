MCU = Atmega328p
F_CPU = 16000000UL


main.o: main.c
  avr-gcc -$(MMCU) -$(F_CPU) -c main.c -o main.o

main.elf: main.o
	avr-gcc -mmcu=$(MCU) -l main.o