# Makefile for HEMA Jig Firmware
# PIC18F4550 Microcontroller
# 
# Requires: MPLAB XC8 Compiler
# Download from: https://www.microchip.com/mplab/compilers

# Compiler and tools
CC = xc8
OBJCOPY = objcopy
HEX2BIN = hex2bin

# Target microcontroller
MCU = 18F4550

# Project name
PROJECT = hema_jig_firmware

# Source files
SOURCES = main.c uart.c jig_control.c

# Header files
HEADERS = config.h pin_definitions.h uart.h jig_control.h

# Object files
OBJECTS = $(SOURCES:.c=.p1)

# Output files
HEX = $(PROJECT).hex
ELF = $(PROJECT).elf
COF = $(PROJECT).cof
MAP = $(PROJECT).map

# Compiler flags
CFLAGS = --chip=$(MCU) \
         --opt=default,+asm,-asmfile,+speed,-space,-debug \
         --output=default,-inhx032 \
         --runtime=default,+clear,+init,-keep,-no_startup,+osccal,-resetbits,-download,-stackcall,+clib \
         --warn=3 \
         --double=24 \
         --float=24

# Linker flags
LDFLAGS = --chip=$(MCU) \
          --output=default,-inhx032 \
          --runtime=default,+clear,+init,-keep,-no_startup,+osccal,-resetbits,-download,-stackcall,+clib

# Build targets
.PHONY: all clean program help

all: $(HEX)
	@echo "Build complete: $(HEX)"
	@echo "Program size:"
	@echo "============="
	@ls -lh $(HEX)

$(HEX): $(OBJECTS)
	$(CC) $(LDFLAGS) -o$(PROJECT) $(OBJECTS)
	@echo "Linking complete"

%.p1: %.c $(HEADERS)
	$(CC) $(CFLAGS) --pass1 $<

clean:
	rm -f *.p1 *.d *.pre *.sym *.cmf *.cof *.hxl *.lst *.obj *.rlf *.sdb
	rm -f $(HEX) $(ELF) $(COF) $(MAP)
	rm -f funclist
	@echo "Clean complete"

# Programming targets (requires PICkit3 or similar programmer)
program: $(HEX)
	@echo "Programming PIC18F4550..."
	@echo "Please use MPLAB IPE or pk2cmd to program:"
	@echo "  pk2cmd -P PIC18F4550 -F $(HEX) -M -R"

# Help target
help:
	@echo "HEMA Jig Firmware Build System"
	@echo "==============================="
	@echo ""
	@echo "Targets:"
	@echo "  all      - Build firmware (default)"
	@echo "  clean    - Remove build artifacts"
	@echo "  program  - Program device (requires programmer)"
	@echo "  help     - Show this help message"
	@echo ""
	@echo "Requirements:"
	@echo "  - MPLAB XC8 Compiler (free mode is sufficient)"
	@echo "  - PIC18F4550 microcontroller"
	@echo "  - PICkit3 or compatible programmer"
	@echo ""
	@echo "Installation:"
	@echo "  1. Install MPLAB XC8 from Microchip website"
	@echo "  2. Add XC8 bin directory to PATH"
	@echo "  3. Run 'make all' to build"
	@echo "  4. Run 'make program' to flash (or use MPLAB IPE)"
	@echo ""
	@echo "Hardware Configuration:"
	@echo "  - PIC18F4550 @ 48MHz (20MHz crystal with PLL)"
	@echo "  - UART: 115200 baud on RC6/RC7"
	@echo "  - GPIO: See pin_definitions.h for details"
	@echo ""
