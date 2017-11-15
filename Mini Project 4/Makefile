#!/usr/bin/make -f

# Ensure consistent environment
SHELL := /bin/bash
PATH := /opt/york/cs/net/bin:$(PATH)

# Path to the GCC toolbox
PKG=/opt/york/cs/net

# Specify the commands needed from the tool chain, CC is the C-compiler,
# OBJCOPY converts the resulting program binary into a format we can load
# into the MBED board
ARCH=arm-none-eabi
CC=$(ARCH)-gcc
OBJCOPY=$(ARCH)-objcopy

# Due to a change in 2012 Linux, we now need to set a user name variable in the
# Makefile so the output path the make install users is right, this is done as
# follows
# USER:=$(shell whoami)

SOURCERY=$(PKG)/sourcery-g++-lite-arm-eabi-2010.09.51-i686-1
GNU_VERSION=4.5.0
THUMB2GNULIB=$(SOURCERY)/$(ARCH)/lib/$(GNU_VERSION)/thumb2
THUMB2GNULIB2=$(SOURCERY)/$(ARCH)/lib/thumb2

# "Cortex Microcontroller Software Interface Standard" Startup files, also the
# flags passed to the C compiler, and linker
CMSIS=$(PKG)/lpc1700-cmsis-lite-2011.01.26-i686-1
CMSISINCLUDES=-I$(CMSIS)/include
CMSISFL=$(CMSIS)/lib/core_cm3.o \
	$(CMSIS)/lib/system_LPC17xx.o \
	$(CMSIS)/lib/startup_LPC17xx.o
LDSCRIPT = $(CMSIS)/lib/ldscript_rom_gnu.ld

CFLAGS=-mcpu=cortex-m3  -mthumb  -Wall  -O0  -mapcs-frame  -D__thumb2__=1 \
  -msoft-float  -gdwarf-2  -mno-sched-prolog  -fno-hosted  -mtune=cortex-m3 \
  -march=armv7-m  -mfix-cortex-m3-ldrd   -ffunction-sections  -fdata-sections \
          -D__RAM_MODE__=0 $(CMSISINCLUDES) -I.

LDFLAGS=$(CMSISFL) -static -mcpu=cortex-m3 -mthumb -mthumb-interwork \
	   -Wl,--start-group -L$(THUMB2GNULIB) -L$(THUMB2GNULIB2) \
           -lc -lg -lstdc++ -lsupc++  -lgcc -lm  -Wl,--end-group \
	   -Xlinker -Map -Xlinker bin/lpc1700.map -Xlinker -T $(LDSCRIPT)

LDFLAGS+=-L$(CMSIS)/lib -lDriversLPC17xxgnu

# Name of the binary being built
EXECNAME	= bin/project4

# Source files provided by the user to build the project
OBJ		= project4.o

# Commands handled by this makefile
all: 	build
	@echo "Build finished"

build: $(OBJ)
	mkdir -p bin # prevent error "No such file or directory" during linking
	$(CC) -o $(EXECNAME) $(OBJ) $(LDFLAGS)
	$(OBJCOPY) -I elf32-little -O binary $(EXECNAME) $(EXECNAME).bin

# make clean - Clean out the source tree ready to re-build the project
clean:
	rm -f `find . | grep \~`
	rm -f *.swp *.o */*.o */*/*.o  *.log
	rm -f *.d */*.d *.srec */*.a bin/*.map
	rm -f *.elf *.wrn bin/*.bin log *.hex
	rm -f $(EXECNAME)

# make install - Installs the resulting binary file to the MBED board, remember
# to sync the file systems, so the copy finishes
install:
	@echo "Copying " $(EXECNAME) "to the MBED file system"
	cp $(EXECNAME).bin /media/kmo507/MBED &
	sync
	@echo "Now press the reset button on all MBED file systems"
