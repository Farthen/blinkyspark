DEVICE     = attiny85           # See avr-help for all possible devices
F_CPU      = 16500000L           # 16Mhz
OBJECTS    = usbdrv/usbdrv.o usbdrv/usbdrvasm.o usbdrv/oddebug.o ws2812/light_ws2812.o led.o main.o # Add more objects for each .c file here

CFLAGS	= $(CPPFLAGS) -std=gnu99 -Os -g -Wall -DF_CPU=$(F_CPU) -mmcu=$(DEVICE) -Iws2812 -IFastLED -Iusbdrv -I. -DDEBUG_LEVEL=0

UPLOAD = micronucleus --run
CC = avr-gcc $(CFLAGS)
CXX = avr-g++ $(CFLAGS)

# symbolic targets:
all:	main.hex

# Generic rule for compiling C files:
.c.o:
	$(CC) -c $< -o $@

# Generic rule for compiling C++ files:
.cpp.o:
	$(CXX) -c $< -o $@

# Generic rule for assembling Assembler source files:
.S.o:
	$(CC) -x assembler-with-cpp -c $< -o $@
	
# Generic rule for compiling C to assembler, used for debugging only.
.c.s:
	$(CC) -S $< -o $@

flash:	all
	python3 ./py/usbclient.py reset
	$(UPLOAD) main.hex

clean:
	rm -f main.hex main.elf main.obj main.cof main.list main.map main.eep.hex main.elf main.s usbdrv/oddebug.s usbdrv/usbdrv.s
	rm -f $(OBJECTS)


main.elf: $(OBJECTS)
	$(CXX) -o main.elf $(OBJECTS)

main.hex: main.elf
	rm -f main.hex
	avr-objcopy -j .text -j .data -O ihex main.elf main.hex
	avr-size --format=avr --mcu=$(DEVICE) main.elf
