#####################
# OpenEBTS library
#

# To keep the paths short we go relative to where most of the source files are
LOCAL_PATH := $(call my-dir)/../../../OpenEBTS

include $(CLEAR_VARS)

LOCAL_MODULE     := OpenEBTS

LOCAL_C_INCLUDES := ../../.. \
                    ../../../OpenEBTS \
                    ../../../libs/NBISWSQ/include \
                    libfreeimage

# For stdstirng.h
LOCAL_CPPFLAGS   += -DSS_NO_STDWCHAR

#
#
#                   libopenebts/Curl.cpp\
#
#

LOCAL_SRC_FILES  := Java/OpenEBTSJNI.c \
                    Java/OpenEBTSJNIHelpers.c \
                    Common.cpp \
                    FreeImageHelpers.cpp \
                    IWNist.cpp \
                    IWNistImp.cpp \
                    IWTransaction.cpp \
                    IWVerification.cpp \
                    NISTField.cpp \
                    NISTRecord.cpp \
                    RuleObj.cpp \
                    TransactionDef.cpp

