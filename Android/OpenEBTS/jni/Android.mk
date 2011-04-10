###############################################################################
# Regular 'Android.mk' file but for simplicity the LOCAL_MODULE,
# LOCAL_C_INCLUDES and LOCAL_SRC_FILES sections are isolated in different
# files, which helps when trying to build just a subset for troubleshooting.
#

#
# NBIS WSQ as a static library
#
include Android_libnbiswsq.mk
include $(BUILD_STATIC_LIBRARY)

#
# FreeImage as a static library
#
include Android_libfreeimage.mk
include $(BUILD_STATIC_LIBRARY)

#
# OpenEBTS as a shared library
#
include Android_libopenebts.mk
LOCAL_STATIC_LIBRARIES := nbiswsq freeimage
LOCAL_LDLIBS := -llog
include $(BUILD_SHARED_LIBRARY)
