#Based on https://github.com/nitsky/stm32-example 
#Modified by Dan Green http://github.com/4ms

BUILDDIR = build

DEVICE = stm32/device
CORE = stm32/core
PERIPH = stm32/periph

STARTUP = startup_stm32f429_439xx

SOURCES +=	\
		   $(PERIPH)/src/stm32f4xx_adc.c \
		   $(PERIPH)/src/stm32f4xx_dac.c \
		   $(PERIPH)/src/stm32f4xx_dma.c \
		   $(PERIPH)/src/stm32f4xx_exti.c \
		   $(PERIPH)/src/stm32f4xx_fmc.c \
		   $(PERIPH)/src/stm32f4xx_gpio.c \
		   $(PERIPH)/src/stm32f4xx_i2c.c \
		   $(PERIPH)/src/stm32f4xx_rcc.c \
		   $(PERIPH)/src/stm32f4xx_spi.c \
		   $(PERIPH)/src/stm32f4xx_syscfg.c \
		   $(PERIPH)/src/stm32f4xx_tim.c \
		   $(PERIPH)/src/misc.c

SOURCES += $(DEVICE)/$(STARTUP).s
SOURCES += $(DEVICE)/system_stm32f4xx.c

SOURCES += main.c codec.c i2s.c \
			adc.c dig_inouts.c dac.c looping_delay.c \
			sdram.c gpiof4.c params.c timekeeper.c resample.c \

OBJECTS = $(addprefix $(BUILDDIR)/, $(addsuffix .o, $(basename $(SOURCES))))

INCLUDES += -I$(DEVICE)/include \
			-I$(CORE)/include \
			-I$(PERIPH)/include \
			-I\

ELF = $(BUILDDIR)/main.elf
HEX = $(BUILDDIR)/main.hex
BIN = $(BUILDDIR)/main.bin

ARCH = arm-none-eabi
CC = $(ARCH)-gcc
LD = $(ARCH)-ld -v
AS = $(ARCH)-as
OBJCPY = $(ARCH)-objcopy
OBJDMP = $(ARCH)-objdump
GDB = $(ARCH)-gdb

 	
#CFLAGS  = -O0 -g -Wall -I.\
#   -mcpu=cortex-m4 -mthumb \
#   -mfpu=fpv4-sp-d16 -mfloat-abi=hard \
#   $(INCLUDES) -DUSE_STDPERIPH_DRIVER


CFLAGS = -g2 -O0 -mlittle-endian -mthumb 
CFLAGS +=  -I. -DARM_MATH_CM4 -D'__FPU_PRESENT=1'  $(INCLUDES)  -DUSE_STDPERIPH_DRIVER
CFLAGS += -mcpu=cortex-m4 -mfloat-abi=hard
CFLAGS +=  -mfpu=fpv4-sp-d16 -fsingle-precision-constant -Wdouble-promotion 

AFLAGS  = -mlittle-endian -mthumb -mcpu=cortex-m4 

LDSCRIPT = $(DEVICE)/stm32f429xx.ld
#LDFLAGS += -T$(LDSCRIPT) -mthumb -mcpu=cortex-m4 -nostdlib
LFLAGS  = -Map main.map -nostartfiles -T $(LDSCRIPT)


all: Makefile $(BIN)

$(BIN): $(ELF)
	$(OBJCPY) -O binary $< $@
	$(OBJDMP) -x --syms $< > $(addsuffix .dmp, $(basename $<))
	ls -l $@ $<

$(HEX): $(ELF)
	$(OBJCPY) --output-target=ihex $< $@

$(ELF): $(OBJECTS)
	$(LD) $(LFLAGS) -o $@ $(OBJECTS)


$(BUILDDIR)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) $< -o $@


$(BUILDDIR)/%.o: %.s
	mkdir -p $(dir $@)
	$(AS) $(AFLAGS) $< -o $@ > $(addprefix $(BUILDDIR)/, $(addsuffix .lst, $(basename $<)))
#	$(CC) -c $(CFLAGS) $< -o $@


flash: $(BIN)
	st-flash write $(BIN) 0x8000000

clean:
	rm -rf build
