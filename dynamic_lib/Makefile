#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITARM)),)
	$(error "DEVKITARM variable not defined")
endif

ifeq ($(strip $(SM64DSe)),)
	$(error "SM64DSe variable not defined")
endif

TARGET   := newcode
# BUILD    := build

#---------------------------------------------------------------------------------
# path to tools
#---------------------------------------------------------------------------------
export PORTLIBS := $(DEVKITPRO)/portlibs/arm
export PATH     := $(DEVKITARM)/bin:$(PORTLIBS)/bin:$(PATH)
LIBNDS          := $(DEVKITPRO)/libnds

#---------------------------------------------------------------------------------
# the prefix on the compiler executables
#---------------------------------------------------------------------------------
PREFIX := arm-none-eabi-

export CC      := $(PREFIX)gcc
export CXX     := $(PREFIX)g++
export AS      := $(PREFIX)as
export AR      := $(PREFIX)ar
export OBJCOPY := $(PREFIX)objcopy
export OBJDUMP := $(PREFIX)objdump
export LD      := $(PREFIX)ld

ifndef SOURCE

SOURCES := $(notdir $(wildcard source/*))
ROM := "$(wildcard ../*.nds)"

all: $(SOURCES)

$(SOURCES):
	@echo building DL $@
	@mkdir -p $(CURDIR)/build/$@
	@echo -e "all:\n\t@make --no-print-directory -C $(CURDIR) -f $(CURDIR)/Makefile" > $(CURDIR)/build/$@/Makefile
	"$(SM64DSe)" compile DL $(ROM) $(CURDIR)/build/$@ $(file < source/$@/target.txt) --env SOURCE=$@ BUILD=build/$@

.PHONY: clean

clean:
	@echo clean ...
	@rm -fr build

else

#---------------------------------------------------------------------------------
# TARGET is the name of the output
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# INCLUDES is a list of directories containing extra header files

#---------------------------------------------------------------------------------
SOURCES  := ../glibc source/$(SOURCE)
INCLUDES := ../include ../source

ARCHFLAGS := -march=armv5te -mtune=arm946e-s

CFLAGS := -Wall -Wextra -Werror -Wno-unused-parameter -Wno-narrowing \
	-Wno-parentheses -Wno-volatile -Wno-invalid-offsetof -Wno-char-subscripts -Wno-trigraphs \
	-Os $(ARCHFLAGS) -fomit-frame-pointer -fwrapv \
	$(INCLUDE) -DARM9 -c

CXXFLAGS := $(CFLAGS) -std=c++23 -fno-exceptions -fno-rtti -fno-threadsafe-statics -faligned-new=4

LDFLAGS = --gc-sections -T $(CURDIR)/../../../symbols.x -T $(CURDIR)/../../../linker.x -Map $(TARGET).map

ifdef CODEADDR
	LDFLAGS += -Ttext $(CODEADDR)
endif

LIBS := 
LIBDIRS := $(LIBNDS)  $(DEVKITARM) $(DEVKITARM)/arm-none-eabi

ifeq (dynamic_lib,$(notdir $(CURDIR)))

export OUTPUT := $(CURDIR)/$(BUILD)/$(TARGET)
export VPATH := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir))
export DEPSDIR := $(CURDIR)/$(BUILD)

CFILES   := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES   := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))

export OFILES := $(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)
export LIBPATHS := $(foreach dir,$(LIBDIRS),-L$(dir)/lib) -L$(DEVKITARM)/lib/gcc/arm-none-eabi/11.1.0

export INCLUDE := $(foreach dir,$(INCLUDES),-iquote $(CURDIR)/$(dir)) \
                  $(foreach dir,$(LIBDIRS),-I$(dir)/include) -I$(CURDIR)/$(BUILD)

.PHONY: $(BUILD)

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@make --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

else

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------

all: $(OUTPUT).bin $(OUTPUT).sym

$(OUTPUT).bin : $(OUTPUT).elf
	$(OBJCOPY) -O binary $< $@
	@echo built ... $(notdir $@)

$(OUTPUT).sym : $(OUTPUT).elf
	$(OBJDUMP) -t $< > $@
	@echo written the symbol table ... $(notdir $@)

#---------------------------------------------------------------------------------
$(OUTPUT).elf: $(OFILES)
	@echo linking $(notdir $@)
	$(LD) $(LDFLAGS) $(OFILES) $(LIBPATHS) $(LIBS) -o $@

#---------------------------------------------------------------------------------
%.o: %.cpp
	@echo $(notdir $<)
	$(CXX) -MMD -MP -MF $(DEPSDIR)/$*.d $(CXXFLAGS) -c $< -o $@ $(ERROR_FILTER)

#---------------------------------------------------------------------------------
%.o: %.c
	@echo $(notdir $<)
	$(CC) -MMD -MP -MF $(DEPSDIR)/$*.d $(CFLAGS) -c $< -o $@ $(ERROR_FILTER)

#---------------------------------------------------------------------------------
%.o: %.s
	@echo $(notdir $<)
	$(CC) -MMD -MP -MF $(DEPSDIR)/$*.d -x assembler-with-cpp $(ARCHFLAGS) -c $< -o $@ $(ERROR_FILTER)

#---------------------------------------------------------------------------------

-include $(DEPSDIR)/*.d

#---------------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------------

endif
