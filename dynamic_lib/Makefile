#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

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
	"$(SM64DSe)" insertDL $(ROM) build targets.txt

$(SOURCES):
	@echo building DL \'$@\' ...
	@mkdir -p $(CURDIR)/build/$@
	@make --no-print-directory SOURCE=$@ BUILD=build/$@

.PHONY: clean

clean:
	@echo clean ...
	@rm -fr build

else

#---------------------------------------------------------------------------------
# path to tools
#---------------------------------------------------------------------------------
PORTLIBS := $(DEVKITPRO)/portlibs/arm
PATH     := $(DEVKITARM)/bin:$(PORTLIBS)/bin:$(PATH)
LIBNDS   := $(DEVKITPRO)/libnds

#---------------------------------------------------------------------------------
# the prefix on the compiler executables
#---------------------------------------------------------------------------------
PREFIX := arm-none-eabi-

CC      := $(PREFIX)gcc
CXX     := $(PREFIX)g++
AS      := $(PREFIX)as
AR      := $(PREFIX)ar
OBJCOPY := $(PREFIX)objcopy
OBJDUMP := $(PREFIX)objdump
LD      := $(PREFIX)ld

#---------------------------------------------------------------------------------
# TARGET is the name of the output
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# INCLUDES is a list of directories containing extra header files

#---------------------------------------------------------------------------------
TARGET   := newcode
SOURCES  := ../glibc source/$(SOURCE)
INCLUDES := ../include ../source

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

LDFLAGS = --gc-sections -T $(CURDIR)/../symbols.x -T $(CURDIR)/../linker.x

export VPATH := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir))
DEPSDIR := $(CURDIR)/$(BUILD)

CFILES   := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES   := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))

OFILENAMES := $(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)
OFILES = $(foreach name,$(OFILENAMES),$(BUILD)/$(name))

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------

OUTPUT0 := $(CURDIR)/$(BUILD)/$(TARGET)
OUTPUT1 := $(CURDIR)/$(BUILD)/$(TARGET)1

BINFILES = $(foreach output,$(OUTPUT0) $(OUTPUT1),$(output).bin)
SYMFILES = $(foreach output,$(OUTPUT0) $(OUTPUT1),$(output).sym)

ELFFILES0 = $(foreach output,$(OUTPUT0),$(output).elf)
ELFFILES1 = $(foreach output,$(OUTPUT1),$(output).elf)

all: $(BINFILES) $(SYMFILES)

$(BINFILES) : $(ELFFILES0) $(ELFFILES1)
	$(OBJCOPY) -O binary $< $@
	@echo built ... $(notdir $@)

$(SYMFILES) : $(ELFFILES0) $(ELFFILES1)
	$(OBJDUMP) -t $< > $@
	@echo written the symbol table ... $(notdir $@)

#---------------------------------------------------------------------------------

$(ELFFILES0): $(OFILES)
	@echo linking $(notdir $@)
	$(LD) $(LDFLAGS) -Ttext 0x02400000 $(OFILES) $(LIBPATHS) $(LIBS) -o $@

$(ELFFILES1): $(OFILES)
	@echo linking $(notdir $@)
	$(LD) $(LDFLAGS) -Ttext 0x02400004 $(OFILES) $(LIBPATHS) $(LIBS) -o $@

#---------------------------------------------------------------------------------
$(BUILD)/%.o: %.cpp
	@echo $(notdir $<)
	$(CXX) -MMD -MP -MF $(DEPSDIR)/$*.d $(CXXFLAGS) -c $< -o $@ $(ERROR_FILTER)

#---------------------------------------------------------------------------------
$(BUILD)/%.o: %.c
	@echo $(notdir $<)
	$(CC) -MMD -MP -MF $(DEPSDIR)/$*.d $(CFLAGS) -c $< -o $@ $(ERROR_FILTER)

#---------------------------------------------------------------------------------
$(BUILD)/%.o: %.s
	@echo $(notdir $<)
	$(CC) -MMD -MP -MF $(DEPSDIR)/$*.d -x assembler-with-cpp $(ARCHFLAGS) -c $< -o $@ $(ERROR_FILTER)

#---------------------------------------------------------------------------------

-include $(DEPSDIR)/*.d

#---------------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------------
