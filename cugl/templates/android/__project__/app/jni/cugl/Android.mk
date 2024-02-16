LOCAL_PATH  := $(call my-dir)
CURR_DEPTH  := ..
CUGL_OFFSET := __CUGL_PATH__

###########################
#
# CUGL static library
#
###########################
CUGL_MAKE := $(LOCAL_PATH)
CUGL_PATH := $(LOCAL_PATH)/$(CURR_DEPTH)/$(CUGL_OFFSET)
SDL2_PATH := $(CUGL_PATH)/sdlapp

# The sublibraries
include $(CUGL_MAKE)/external/box2d/Android.mk
include $(CUGL_MAKE)/external/poly2tri/Android.mk

include $(CLEAR_VARS)

LOCAL_MODULE := CUGL

LOCAL_C_INCLUDES := $(CUGL_PATH)/include
LOCAL_C_INCLUDES += $(SDL2_PATH)/include
LOCAL_C_INCLUDES += $(CUGL_PATH)/external/poly2tri
LOCAL_C_INCLUDES += $(CUGL_PATH)/external/box2d/include

# Add your application source files here...
LOCAL_PATH := $(CUGL_PATH)/source

LOCAL_SRC_FILES := \
	$(subst $(LOCAL_PATH)/,, \
	$(wildcard $(LOCAL_PATH)/base/*.cpp) \
	$(wildcard $(LOCAL_PATH)/util/*.cpp) \
	$(wildcard $(LOCAL_PATH)/math/*.cpp) \
	$(wildcard $(LOCAL_PATH)/math/dsp/*.cpp) \
	$(wildcard $(LOCAL_PATH)/math/polygon/*.cpp) \
	$(wildcard $(LOCAL_PATH)/input/*.cpp) \
	$(wildcard $(LOCAL_PATH)/input/gestures/*.cpp) \
	$(wildcard $(LOCAL_PATH)/io/*.cpp) \
	$(wildcard $(LOCAL_PATH)/audio/*.cpp) \
	$(wildcard $(LOCAL_PATH)/audio/graph/*.cpp) \
	$(wildcard $(LOCAL_PATH)/render/*.cpp) \
	$(wildcard $(LOCAL_PATH)/assets/*.cpp) \
	$(wildcard $(LOCAL_PATH)/physics2/*.cpp) \
	$(wildcard $(LOCAL_PATH)/scene2/*.cpp) \
	$(wildcard $(LOCAL_PATH)/scene2/graph/*.cpp) \
	$(wildcard $(LOCAL_PATH)/scene2/ui/*.cpp) \
	$(wildcard $(LOCAL_PATH)/scene2/layout/*.cpp) \
	$(wildcard $(LOCAL_PATH)/external/*.c) \
	$(wildcard $(LOCAL_PATH)/external/*.cpp))

LOCAL_CFLAGS += -DGL_GLEXT_PROTOTYPES
LOCAL_CFLAGS += \
	-Wno-missing-prototypes \
	-Wno-implicit-const-int-float-conversion \

LOCAL_STATIC_LIBRARIES := poly2tri
LOCAL_STATIC_LIBRARIES += box2d

LOCAL_SHARED_LIBRARIES := SDL2
LOCAL_SHARED_LIBRARIES += SDL2_app
LOCAL_SHARED_LIBRARIES += SDL2_atk
LOCAL_SHARED_LIBRARIES += SDL2_image
LOCAL_SHARED_LIBRARIES += SDL2_ttf

include $(BUILD_STATIC_LIBRARY)
