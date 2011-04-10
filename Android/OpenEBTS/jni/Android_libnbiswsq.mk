####################
# NBIS WSQ library
#

LOCAL_PATH := $(call my-dir)/../../../libs/NBISWSQ

include $(CLEAR_VARS)

LOCAL_MODULE     := nbiswsq

LOCAL_C_INCLUDES := ../../../libs/NBISWSQ \
                    ../../../libs/NBISWSQ/include

LOCAL_CFLAGS     += -D__NBISLE__

LOCAL_SRC_FILES  := fet/allocfet.c \
                    fet/delfet.c \
                    fet/extrfet.c \
                    fet/freefet.c \
                    fet/lkupfet.c \
                    fet/nistcom.c \
                    fet/strfet.c \
                    fet/updatfet.c \
                    ioutil/dataio.c \
                    ioutil/filesize.c \
                    jpegl/huftable.c \
                    jpegl/_huff.c \
                    jpegl/_tableio.c \
                    util/computil.c \
                    util/fatalerr.c \
                    util/syserr.c \
                    wsq/decoder.c \
                    wsq/encoder.c \
                    wsq/globals.c \
                    wsq/huff.c \
                    wsq/ppi.c \
                    wsq/tableio.c \
                    wsq/tree.c \
                    wsq/util.c
