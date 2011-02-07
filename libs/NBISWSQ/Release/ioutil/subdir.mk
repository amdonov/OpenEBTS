################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../ioutil/dataio.c \
../ioutil/filesize.c 

OBJS += \
./ioutil/dataio.o \
./ioutil/filesize.o 

C_DEPS += \
./ioutil/dataio.d \
./ioutil/filesize.d 


# Each subdirectory must supply rules for building sources it contributes
ioutil/%.o: ../ioutil/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -D__NBISLE__ -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


