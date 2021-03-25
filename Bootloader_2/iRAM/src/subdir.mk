################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/CAN.c \
../src/FLASH.c \
../src/MAIN.c \
../src/lz4.c 

OBJS += \
./src/CAN.o \
./src/FLASH.o \
./src/MAIN.o \
./src/lz4.o 

C_DEPS += \
./src/CAN.d \
./src/FLASH.d \
./src/MAIN.d \
./src/lz4.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: TriCore C Compiler'
	"$(TRICORE_TOOLS)/bin/tricore-gcc" -c -I"../h" -fno-common -Os -g3 -W -Wall -Wextra -Wdiv-by-zero -Warray-bounds -Wcast-align -Wignored-qualifiers -Wformat -Wformat-security -pipe -DTRIBOARD_TC1791 -fshort-double -mcpu=tc1791 -mversion-info -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


