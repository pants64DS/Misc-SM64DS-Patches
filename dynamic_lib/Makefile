# Disable default suffixes
.SUFFIXES:

TARGET_LO := newcode_lo
TARGET_HI := newcode_hi

ifndef SOURCE

ifeq ($(strip $(DEVKITARM)),)
	$(error "DEVKITARM variable not defined")
endif

ifeq ($(strip $(SM64DSe)),)
	$(error "SM64DSe variable not defined")
endif

SOURCES := $(notdir $(wildcard source/*))
ROM := "$(wildcard ../*.nds)"

all: $(SOURCES)
	"$(SM64DSe)" insertDLs --rom=$(ROM) build targets.txt

# Call make recursively for each DL
$(SOURCES):
	@echo building DL \'$@\' ...
	@mkdir -p $(CURDIR)/build/$@
	@make --no-print-directory SOURCE=$@ BUILD=build/$@

.PHONY: clean

clean:
	@echo clean ...
	@rm -fr build

else
# Now SOURCE is the source folder of a specific DL

PORTLIBS := $(DEVKITPRO)/portlibs/arm
PATH     := $(DEVKITARM)/bin:$(PORTLIBS)/bin:$(PATH)
LIBNDS   := $(DEVKITPRO)/libnds

PREFIX := arm-none-eabi-

CC      := $(PREFIX)gcc
CXX     := $(PREFIX)g++
AS      := $(PREFIX)as
AR      := $(PREFIX)ar
OBJCOPY := $(PREFIX)objcopy
OBJDUMP := $(PREFIX)objdump
LD      := $(PREFIX)ld

SOURCES  := ../libc source/$(SOURCE)
INCLUDES := ../include ../source ../SM64DS-PI/include

LIBS := 
LIBDIRS := $(LIBNDS)  $(DEVKITARM) $(DEVKITARM)/arm-none-eabi

LIBPATHS := $(foreach dir,$(LIBDIRS),-L$(dir)/lib) -L$(DEVKITARM)/lib/gcc/arm-none-eabi/11.1.0
INCLUDE  := $(foreach dir,$(INCLUDES),-iquote $(CURDIR)/$(dir)) \
            $(foreach dir,$(LIBDIRS),-I$(dir)/include) -I$(CURDIR)/$(BUILD)

ARCHFLAGS := -march=armv5te -mtune=arm946e-s

CFLAGS := -Wall -Wextra -Werror -Wno-unused-parameter -Wno-narrowing \
	-Wno-parentheses -Wno-volatile -Wno-invalid-offsetof -Wno-char-subscripts -Wno-trigraphs \
	-Os $(ARCHFLAGS) -fomit-frame-pointer -fwrapv \
	$(INCLUDE) -DARM9 -c

CXXFLAGS := $(CFLAGS) -std=c++23 -fno-exceptions -fno-rtti -fno-threadsafe-statics -faligned-new=4

SYMBOLS = $(CURDIR)/../SM64DS-PI/symbols9.x
LDFLAGS = --gc-sections -T $(SYMBOLS) -T $(CURDIR)/../linker.x

export VPATH := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir))
DEPSDIR := $(CURDIR)/$(BUILD)

CFILES   := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES   := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))

OFILENAMES := $(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)
OFILES = $(foreach name,$(OFILENAMES),$(BUILD)/$(name))

OUTPUT_LO := $(CURDIR)/$(BUILD)/$(TARGET_LO)
OUTPUT_HI := $(CURDIR)/$(BUILD)/$(TARGET_HI)

BINFILES = $(OUTPUT_LO).bin $(OUTPUT_HI).bin
SYMFILES = $(OUTPUT_LO).sym $(OUTPUT_HI).sym

all: $(BINFILES) $(SYMFILES)

$(CURDIR)/$(BUILD)/%.bin : $(CURDIR)/$(BUILD)/%.elf
	$(OBJCOPY) -O binary $< $@
	@echo built ... $(notdir $@)

$(CURDIR)/$(BUILD)/%.sym : $(CURDIR)/$(BUILD)/%.elf
	$(OBJDUMP) -t $< > $@
	@echo written the symbol table ... $(notdir $@)

$(OUTPUT_LO).elf: $(OFILES)
	@echo linking $(notdir $@)
	$(LD) $(LDFLAGS) -Ttext 0x02400000 $(OFILES) $(LIBPATHS) $(LIBS) -o $@

$(OUTPUT_HI).elf: $(OFILES)
	@echo linking $(notdir $@)
	$(LD) $(LDFLAGS) -Ttext 0x02400004 $(OFILES) $(LIBPATHS) $(LIBS) -o $@

$(BUILD)/%.o: %.cpp
	@echo $(notdir $<)
	$(CXX) -MMD -MP -MF $(DEPSDIR)/$*.d $(CXXFLAGS) -c $< -o $@ $(ERROR_FILTER)

$(BUILD)/%.o: %.c
	@echo $(notdir $<)
	$(CC) -MMD -MP -MF $(DEPSDIR)/$*.d $(CFLAGS) -c $< -o $@ $(ERROR_FILTER)

$(BUILD)/%.o: %.s
	@echo $(notdir $<)
	$(CC) -MMD -MP -MF $(DEPSDIR)/$*.d -x assembler-with-cpp $(ARCHFLAGS) -c $< -o $@ $(ERROR_FILTER)

-include $(DEPSDIR)/*.d

endif
