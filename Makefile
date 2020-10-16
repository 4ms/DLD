BINARYNAME = main

STARTUP = startup_stm32f427_modern.s
SYSTEM = system_stm32f4xx.c
LOADFILE = stm32f427_modern.ld

DEVICE = stm32/device
CORE = stm32/core
PERIPH = stm32/periph

BUILDDIR = build

SOURCES += $(wildcard $(PERIPH)/src/*.c)
SOURCES += $(DEVICE)/src/$(STARTUP)
SOURCES += $(DEVICE)/src/$(SYSTEM)
SOURCES += $(wildcard src/*.c)
SOURCES += $(wildcard libhwtests/src/*.c)
SOURCES += $(wildcard libhwtests/src/*.cc)
SOURCES += $(wildcard libhwtests/src/*.cpp)
SOURCES += $(wildcard hardware_tests/src/*.c)
SOURCES += $(wildcard hardware_tests/src/*.cc)
SOURCES += $(wildcard hardware_tests/src/*.cpp)

OBJECTS = $(addprefix $(BUILDDIR)/, $(addsuffix .o, $(basename $(SOURCES))))
DEPS = $(OBJECTS:.o=.d)

# show:
# 	echo $(OBJECTS)

INCLUDES += -I$(DEVICE)/include \
			-I$(CORE)/include \
			-I$(PERIPH)/include \
			-Iinc \
			-Ilibhwtests/inc \
			-Ihardware_tests/inc \

ELF = $(BUILDDIR)/$(BINARYNAME).elf
HEX = $(BUILDDIR)/$(BINARYNAME).hex
BIN = $(BUILDDIR)/$(BINARYNAME).bin

CCACHE = ccache
ARCH = arm-none-eabi
CC = $(CCACHE) $(ARCH)-gcc
CXX = $(CCACHE) $(ARCH)-g++
LD = $(ARCH)-g++
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
          -finline-functions -funswitch-loops -fpredictive-commoning -fgcse-after-reload -ftree-vectorize \

# Causes Freeze on run: -fschedule-insns  -fschedule-insns2 

CFLAGS = -g2
CFLAGS += -mlittle-endian -mthumb 
CFLAGS += -mcpu=cortex-m4 
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16 
CFLAGS += -DARM_MATH_CM4 -D'__FPU_PRESENT=1' -DUSE_STDPERIPH_DRIVER
CFLAGS += -I. $(INCLUDES)
CFLAGS += -fno-exceptions -fsingle-precision-constant -Wdouble-promotion
CFLAGS += -ffreestanding
CFLAGS += --specs=nosys.specs

CXXFLAGS = -std=c++17
CXXFLAGS += -Wno-register

AFLAGS  = -mlittle-endian -mthumb -mcpu=cortex-m4

LDSCRIPT = $(DEVICE)/$(LOADFILE)
LFLAGS  = $(CFLAGS) -Wl,-Map,main.map -T $(LDSCRIPT)

# $(BUILDDIR)/hardware_tests/src/hardware_test_gates.o: OPTFLAGS = -O0
# $(BUILDDIR)/libhwtests/src/GateOutChecker.o: OPTFLAGS = -O0
# $(BUILDDIR)/libhwtests/src/GateInChecker.o: OPTFLAGS = -O0
# $(BUILDDIR)/hardware_tests/src/hardware_test_switches_buttons.o: OPTFLAGS = -O0
# $(BUILDDIR)/libhwtests/src/ButtonChecker.o: OPTFLAGS = -O0
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
	@$(CXX) -c $(DEPFLAGS) $(OPTFLAGS) $(CFLAGS) $(CXXFLAGS) $< -o $@

$(BUILDDIR)/%.o: %.cpp $(BUILDDIR)/%.d
	@mkdir -p $(dir $@)
	@echo "Compiling:" $<
	@$(CXX) -c $(DEPFLAGS) $(OPTFLAGS) $(CFLAGS) $(CXXFLAGS) $< -o $@

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
