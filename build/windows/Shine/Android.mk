BUILD_PATH := $(call my-dir)
SDL2_PATH  := $(BUILD_PATH)/../../../..

###########################
#
# SDL_Codec shared library
#
###########################
CODEC_PATH := $(SDL2_PATH)/extensions/codec

include $(CLEAR_VARS)

LOCAL_MODULE := SDL2_codec

LOCAL_C_INCLUDES := $(SDL2_PATH)/include
LOCAL_C_INCLUDES += $(CODEC_PATH)
LOCAL_C_INCLUDES += $(CODEC_PATH)/ogg/include
LOCAL_C_INCLUDES += $(CODEC_PATH)/flac/include
LOCAL_C_INCLUDES += $(CODEC_PATH)/flac/src/libFLAC/include
LOCAL_C_INCLUDES += $(CODEC_PATH)/vorbis/include
LOCAL_C_INCLUDES += $(CODEC_PATH)/vorbis/lib
LOCAL_C_INCLUDES += $(CODEC_PATH)/minimp3


LOCAL_C_FLAGS += -fkeep-inline-functions
ifeq ($(TARGET_ARCH_ABI),armeabi)
	LOCAL_CFLAGS += -D_ARM_ASSEM_
endif
ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
	LOCAL_CFLAGS += -D_ARM_ASSEM_
endif

LOCAL_CFLAGS    += -DFLAC__HAS_OGG

# To keep file mangling to a minimum
LOCAL_PATH = $(CODEC_PATH)

# CODEC SOURCE CODE
LOCAL_SRC_FILES := \
	CODEC_common.c \
	CODEC_vorbis.c \
	CODEC_flac.c \
	CODEC_mpeg.c \
	CODEC_wav.c
LOCAL_STATIC_LIBRARIES :=


# FLAC SOURCE CODE
LOCAL_SRC_FILES += \
	flac/src/libFLAC/bitmath.c \
	flac/src/libFLAC/bitreader.c \
	flac/src/libFLAC/bitwriter.c \
	flac/src/libFLAC/cpu.c \
	flac/src/libFLAC/crc.c \
	flac/src/libFLAC/fixed.c \
	flac/src/libFLAC/float.c \
	flac/src/libFLAC/format.c \
	flac/src/libFLAC/lpc.c \
	flac/src/libFLAC/md5.c \
	flac/src/libFLAC/memory.c \
	flac/src/libFLAC/metadata_iterators.c \
	flac/src/libFLAC/metadata_object.c \
	flac/src/libFLAC/ogg_decoder_aspect.c \
	flac/src/libFLAC/ogg_encoder_aspect.c \
	flac/src/libFLAC/ogg_helper.c \
	flac/src/libFLAC/ogg_mapping.c \
	flac/src/libFLAC/stream_decoder.c \
	flac/src/libFLAC/stream_encoder.c \
	flac/src/libFLAC/stream_encoder_framing.c \
	flac/src/libFLAC/window.c
LOCAL_STATIC_LIBRARIES :=

# OGG VORBIS SOURCE CODE
LOCAL_SRC_FILES += \
	vorbis/lib/analysis.c \
    vorbis/lib/bitrate.c \
    vorbis/lib/block.c \
    vorbis/lib/codebook.c \
    vorbis/lib/envelope.c \
    vorbis/lib/floor0.c \
    vorbis/lib/floor1.c \
    vorbis/lib/info.c \
    vorbis/lib/lookup.c \
    vorbis/lib/lpc.c \
    vorbis/lib/lsp.c \
    vorbis/lib/mapping0.c \
    vorbis/lib/mdct.c \
    vorbis/lib/psy.c \
    vorbis/lib/registry.c \
    vorbis/lib/res0.c \
    vorbis/lib/sharedbook.c \
    vorbis/lib/smallft.c \
    vorbis/lib/synthesis.c \
    vorbis/lib/tone.c \
    vorbis/lib/vorbisenc.c \
    vorbis/lib/vorbisfile.c \
    vorbis/lib/window.c \
    ogg/src/framing.c \
	ogg/src/bitwise.c
LOCAL_STATIC_LIBRARIES :=

# MP3 SOURCE CODE
LOCAL_SRC_FILES += minimp3/mp3stream.c

LOCAL_EXPORT_C_INCLUDES += $(LOCAL_C_INCLUDES)

LOCAL_SHARED_LIBRARIES := SDL2

include $(BUILD_SHARED_LIBRARY)

###########################
#
# SDL TTF static library
#
###########################

LOCAL_MODULE := SDL2_codec_static

LOCAL_MODULE_FILENAME := libSDL2_codec

include $(BUILD_STATIC_LIBRARY)
