BINARYNAME = main

STARTUP = startup_stm32f427_437xx.s
SYSTEM = system_stm32f4xx.c
LOADFILE = stm32f427.ld

DEVICE = stm32/device
CORE = stm32/core
PERIPH = stm32/periph

BUILDDIR = build

SOURCES += $(wildcard $(PERIPH)/src/*.c)
SOURCES += $(DEVICE)/src/$(STARTUP)
SOURCES += $(DEVICE)/src/$(SYSTEM)
SOURCES += $(wildcard src/*.c)
SOURCES += $(wildcard lib_hwtest/src/*.c)
SOURCES += $(wildcard lib_hwtest/src/*.cc)
SOURCES += $(wildcard lib_hwtest/src/*.cpp)

OBJECTS = $(addprefix $(BUILDDIR)/, $(addsuffix .o, $(basename $(SOURCES))))
DEPS = $(OBJECTS:.o=.d)

INCLUDES += -I$(DEVICE)/include \
			-I$(CORE)/include \
			-I$(PERIPH)/include \
			-Iinc \
			-Ilib_hwtest/inc \
			-I \

ELF = $(BUILDDIR)/$(BINARYNAME).elf
HEX = $(BUILDDIR)/$(BINARYNAME).hex
BIN = $(BUILDDIR)/$(BINARYNAME).bin

ARCH = arm-none-eabi
CC = $(ARCH)-gcc
LD = $(ARCH)-ld -v
AS = $(ARCH)-as
OBJCPY = $(ARCH)-objcopy
OBJDMP = $(ARCH)-objdump
GDB = $(ARCH)-gdb


OPTFLAGS = -O1 \
          -fthread-jumps \
          -falign-functions  -falign-jumps \
          -falign-loops  -falign-labels \
          -fcaller-saves \
          -fcrossjumping \
          -fcse-follow-jumps  -fcse-skip-blocks \
          -fdelete-null-pointer-checks \
          -fexpensive-optimizations \
          -fgcse  -fgcse-lm  \
          -findirect-inlining \
          -foptimize-sibling-calls \
          -fpeephole2 \
          -fregmove \
          -freorder-blocks  -freorder-functions \
          -frerun-cse-after-loop  \
          -fsched-interblock  -fsched-spec \
          -fstrict-aliasing -fstrict-overflow \
          -ftree-switch-conversion \
          -ftree-pre \
          -ftree-vrp \
          -finline-functions -funswitch-loops -fpredictive-commoning -fgcse-after-reload -ftree-vectorize
# Causes Freeze on run: -fschedule-insns  -fschedule-insns2 

CFLAGS = -g2
CFLAGS += -mlittle-endian -mthumb 
CFLAGS +=  -I. -DARM_MATH_CM4 -D'__FPU_PRESENT=1'  $(INCLUDES)  -DUSE_STDPERIPH_DRIVER
CFLAGS += -mcpu=cortex-m4 -mfloat-abi=hard
CFLAGS +=  -mfpu=fpv4-sp-d16 -fsingle-precision-constant -Wdouble-promotion 

AFLAGS  = -mlittle-endian -mthumb -mcpu=cortex-m4 

LDSCRIPT = $(DEVICE)/$(LOADFILE)
LFLAGS  = -Map main.map -nostartfiles -T $(LDSCRIPT)

#$(BUILDDIR)/hardware_test_switches_buttons.o: OPTFLAGS = -O0
#$(BUILDDIR)/hardware_test_adc.o: OPTFLAGS = -O0

DEPFLAGS = -MMD -MP -MF $(BUILDDIR)/$(basename $<).d

all: Makefile $(BIN) $(HEX)

$(BIN): $(ELF)
	@$(OBJCPY) -O binary $< $@
	@$(OBJDMP) -x --syms $< > $(addsuffix .dmp, $(basename $<))
	ls -l $@ $<

$(HEX): $(ELF)
	@$(OBJCPY) --output-target=ihex $< $@

$(ELF): $(OBJECTS)
	@echo "Linking..."
	@$(LD) $(LFLAGS) -o $@ $(OBJECTS)

$(BUILDDIR)/%.o: %.c $(BUILDDIR)/%.d
	@mkdir -p $(dir $@)
	@echo "Compiling:" $<
	@$(CC) -c $(DEPFLAGS) $(OPTFLAGS) $(CFLAGS) $< -o $@

$(BUILDDIR)/%.o: %.cc $(BUILDDIR)/%.d
	@mkdir -p $(dir $@)
	@echo "Compiling:" $<
	@$(CXX) -c $(DEPFLAGS) $(OPTFLAGS) $(CFLAGS) $< -o $@

$(BUILDDIR)/%.o: %.s
	@mkdir -p $(dir $@)
	@echo "Compiling:" $<
	@$(AS) $(AFLAGS) $< -o $@ > $(addprefix $(BUILDDIR)/, $(addsuffix .lst, $(basename $<)))

%.d: ;

ifneq "$(MAKECMDGOALS)" "clean"
-include $(DEPS)
endif

flash: $(BIN)
	st-flash write $(BIN) 0x8008000

clean:
	rm -rf build
	
wav: fsk-wav

qpsk-wav: $(BIN)
	python stm_audio_bootloader/qpsk/encoder.py \
		-t stm32f4 -s 48000 -b 12000 -c 6000 -p 256 \
		$(BIN)

fsk-wav: $(BIN)
	python stm_audio_bootloader/fsk/encoder.py \
		-s 48000 -b 16 -n 8 -z 4 -p 256 -g 16384 -k 1800 \
		$(BIN)
