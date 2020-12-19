################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(I_DEPS)),)
-include $(I_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CPCP_DEPS)),)
-include $(CPCP_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

#rebuild target
rebuild: clean all

# All Target
all: Bootloader_2.elf

# Tool invocations
Bootloader_2.elf: $(OBJS) $(USER_OBJS) $(ASM)
	@echo 'Building target: $@'
	@echo 'Invoking: TriCore C Linker'
	"$(TRICORE_TOOLS)/bin/tricore-gcc" -o  "Bootloader_2.elf" -T"../ld/iRAM.ld" @"iRAM.objectlist" -Wl,--gc-sections -mcpu=tc1791 -Wl,--mem-holes -Wl,--no-warn-flags -Wl,-Map="$(basename $(notdir $@)).map" -Wl,--cref -fshort-double -Wl,-n -Wl,--extmap="a"
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(I_DEPS)$(PCP_S)$(EXECUTABLES)$(OBJS)$(C_DEPS)$(CPCP_DEPS) Bootloader_2.elf
	-@echo ' '

.PHONY: all clean dependents

-include ../makefile.targets
