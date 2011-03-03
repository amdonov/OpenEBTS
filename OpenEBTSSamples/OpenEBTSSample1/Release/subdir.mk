################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../../Helpers.cpp \
../OpenEBTSSample1.cpp 

OBJS += \
./Helpers.o \
./OpenEBTSSample1.o 

CPP_DEPS += \
./Helpers.d \
./OpenEBTSSample1.d 


# Each subdirectory must supply rules for building sources it contributes
Helpers.o: ../../Helpers.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -DUNICODE -I../../../OpenEBTS -I../../.. -I../.. -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -DUNICODE -I../../../OpenEBTS -I../../.. -I../.. -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


