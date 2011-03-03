################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../jpegl/_huff.c \
../jpegl/_tableio.c \
../jpegl/huftable.c 

OBJS += \
./jpegl/_huff.o \
./jpegl/_tableio.o \
./jpegl/huftable.o 

C_DEPS += \
./jpegl/_huff.d \
./jpegl/_tableio.d \
./jpegl/huftable.d 


# Each subdirectory must supply rules for building sources it contributes
jpegl/%.o: ../jpegl/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I../include -D__NBISLE__ -O3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


