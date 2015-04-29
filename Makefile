# Makefile for STM32F4
# 11-10-2011 E. Brombaugh
# 2014-07-25 D Green

#Startup file
#STARTUP = startup_stm32f4xx
STARTUP = startup_stm32f429_439xx

# Object files
OBJECTS = 	$(STARTUP).o system_stm32f4xx.o main.o codec.o i2s.o \
			adc.o dig_inouts.o dac.o looping_delay.o \
			sdram.o gpiof4.o params.o timekeeper.o resample.o \
			stm32f4xx_gpio.o stm32f4xx_i2c.o stm32f4xx_rcc.o \
			stm32f4xx_spi.o stm32f4xx_dma.o stm32f4xx_adc.o misc.o \
			stm32f4xx_dac.o stm32f4xx_tim.o stm32f4xx_exti.o stm32f4xx_syscfg.o \
			stm32f4xx_fmc.o
 
# Linker script
LDSCRIPT = stm32f429xx.ld


CFLAGS = -g2 -O1 -mlittle-endian -mthumb
CFLAGS +=  -I. -DARM_MATH_CM4 -D'__FPU_PRESENT=1' -DUSE_STDPERIPH_DRIVER
CFLAGS += -mcpu=cortex-m4 -mfloat-abi=hard
CFLAGS +=  -mfpu=fpv4-sp-d16 -fsingle-precision-constant -Wdouble-promotion 

AFLAGS  = -mlittle-endian -mthumb -mcpu=cortex-m4 
LFLAGS  = -Map main.map -nostartfiles -T $(LDSCRIPT)

# Executables
ARCH = arm-none-eabi
CC = $(ARCH)-gcc
LD = $(ARCH)-ld -v
AS = $(ARCH)-as
OBJCPY = $(ARCH)-objcopy
OBJDMP = $(ARCH)-objdump
GDB = $(ARCH)-gdb

CPFLAGS = --output-target=binary -j .text -j .data
ODFLAGS	= -x --syms

FLASH = st-flash

# Targets
all: Makefile main.bin

clean:
	-rm -f $(OBJECTS) *.lst *.elf *.bin *.map *.dmp

flash: gdb_flash

stlink_flash: main.bin
	$(FLASH) write main.bin 0x08000000
	
gdb_flash: main.elf
	$(GDB) -x flash_cmd.gdb -batch

disassemble: main.elf
	$(OBJDMP) -dS main.elf > main.dis
	
main.hex: main.elf
	$(OBJCPY) --output-target=ihex main.elf main.hex

main.bin: main.elf 
	$(OBJCPY) $(CPFLAGS) main.elf main.bin
	$(OBJDMP) $(ODFLAGS) main.elf > main.dmp
	ls -l main.elf main.bin

main.elf: $(OBJECTS) $(LDSCRIPT)
	$(LD) $(LFLAGS) -o main.elf $(OBJECTS)

$(STARTUP).o: $(STARTUP).s
	$(AS) $(AFLAGS) $(STARTUP).s -o $(STARTUP).o > $(STARTUP).lst

%.o: %.c %.h
	$(CC) $(CFLAGS) -c -o $@ $<

