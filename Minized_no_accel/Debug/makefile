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
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
ELFSIZE += \
Minized_no_accel.elf.size \


# All Target
all: pre-build main-build

# Main-build Target
main-build: Minized_no_accel.elf secondary-outputs

# Tool invocations
Minized_no_accel.elf: $(OBJS) ../src/lscript.ld $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: ARM v7 gcc linker'
	arm-none-eabi-gcc -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -Wl,-build-id=none -specs=Xilinx.spec -Wl,-T -Wl,../src/lscript.ld -L../../Minized_no_accel_bsp/ps7_cortexa9_0/lib -o "Minized_no_accel.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

Minized_no_accel.elf.size: Minized_no_accel.elf
	@echo 'Invoking: ARM v7 Print Size'
	arm-none-eabi-size Minized_no_accel.elf  |tee "Minized_no_accel.elf.size"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(S_UPPER_DEPS)$(C_DEPS)$(ELFSIZE) Minized_no_accel.elf
	-@echo ' '

pre-build:
	-a9-linaro-pre-build-step
	-@echo ' '

post-build:
	-arm-none-eabi-objcopy -O ihex Minized_no_accel.elf Minized_no_accel.hex
	-@echo ' '

secondary-outputs: $(ELFSIZE)

.PHONY: all clean dependents
.SECONDARY: main-build pre-build post-build

-include ../makefile.targets
