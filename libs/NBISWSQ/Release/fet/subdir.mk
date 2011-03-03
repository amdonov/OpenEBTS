################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../fet/allocfet.c \
../fet/delfet.c \
../fet/extrfet.c \
../fet/freefet.c \
../fet/lkupfet.c \
../fet/nistcom.c \
../fet/strfet.c \
../fet/updatfet.c 

OBJS += \
./fet/allocfet.o \
./fet/delfet.o \
./fet/extrfet.o \
./fet/freefet.o \
./fet/lkupfet.o \
./fet/nistcom.o \
./fet/strfet.o \
./fet/updatfet.o 

C_DEPS += \
./fet/allocfet.d \
./fet/delfet.d \
./fet/extrfet.d \
./fet/freefet.d \
./fet/lkupfet.d \
./fet/nistcom.d \
./fet/strfet.d \
./fet/updatfet.d 


# Each subdirectory must supply rules for building sources it contributes
fet/%.o: ../fet/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -D__NBISLE__ -I../include -O3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


