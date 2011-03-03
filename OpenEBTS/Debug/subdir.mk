################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Common.cpp \
../Curl.cpp \
../FreeImageHelpers.cpp \
../IWNist.cpp \
../IWNistImp.cpp \
../IWTransaction.cpp \
../IWVerification.cpp \
../NISTField.cpp \
../NISTRecord.cpp \
../RuleObj.cpp \
../TransactionDef.cpp 

OBJS += \
./Common.o \
./Curl.o \
./FreeImageHelpers.o \
./IWNist.o \
./IWNistImp.o \
./IWTransaction.o \
./IWVerification.o \
./NISTField.o \
./NISTRecord.o \
./RuleObj.o \
./TransactionDef.o 

CPP_DEPS += \
./Common.d \
./Curl.d \
./FreeImageHelpers.d \
./IWNist.d \
./IWNistImp.d \
./IWTransaction.d \
./IWVerification.d \
./NISTField.d \
./NISTRecord.d \
./RuleObj.d \
./TransactionDef.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -DUNICODE -D_DEBUG -I../.. -I"../../libs/NBISWSQ/include" -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


