LOCAL_PATH  := $(call my-dir)
CURR_DEPTH  := ..
CUGL_OFFSET := ../../../../../cugl

########################
#
# Main Application Entry
#
########################
CUGL_PATH := $(LOCAL_PATH)/$(CURR_DEPTH)/$(CUGL_OFFSET)
SDL2_PATH := $(CUGL_PATH)/sdlapp
PROJ_PATH := $(LOCAL_PATH)/${CURR_DEPTH}/../../../../..

include $(CLEAR_VARS)

LOCAL_MODULE := main
LOCAL_C_INCLUDES := $(CUGL_PATH)/include
LOCAL_C_INCLUDES += $(SDL2_PATH)/include
LOCAL_C_INCLUDES += $(CUGL_PATH)/external/box2d/include
LOCAL_C_INCLUDES += $(CUGL_PATH)/external/poly2tri
LOCAL_C_INCLUDES += $(PROJ_PATH)/source


# Add your application source files here.
LOCAL_PATH = $(PROJ_PATH)
LOCAL_SRC_FILES := $(subst $(LOCAL_PATH)/,, \
	$(LOCAL_PATH)/source/AudioController.cpp \
	$(LOCAL_PATH)/source/InputController.cpp \
	$(LOCAL_PATH)/source/DirtTimer.cpp \
	$(LOCAL_PATH)/source/NetworkController.cpp \
	$(LOCAL_PATH)/source/CollisionController.cpp \
	$(LOCAL_PATH)/source/PlayerCharacter.cpp \
	$(LOCAL_PATH)/source/LevelScene.cpp \
	$(LOCAL_PATH)/source/ProjectileSet.cpp \
	$(LOCAL_PATH)/source/LobbyScene.cpp \
	$(LOCAL_PATH)/source/MenuScene.cpp \
	$(LOCAL_PATH)/source/LoadingScene.cpp \
	$(LOCAL_PATH)/source/Bird.cpp \
	$(LOCAL_PATH)/source/TutorialController.cpp \
	$(LOCAL_PATH)/source/ClientJoinScene.cpp \
	$(LOCAL_PATH)/source/GameScene.cpp \
	$(LOCAL_PATH)/source/WindowGrid.cpp \
	$(LOCAL_PATH)/source/DirtThrowInputController.cpp \
	$(LOCAL_PATH)/source/NetStructs.cpp \
	$(LOCAL_PATH)/source/main.cpp \
	$(LOCAL_PATH)/source/GameplayController.cpp \
	$(LOCAL_PATH)/source/StaticFilth.cpp \
	$(LOCAL_PATH)/source/App.cpp)

# Link in SDL2
LOCAL_SHARED_LIBRARIES := SDL2
LOCAL_SHARED_LIBRARIES += SDL2_image
LOCAL_SHARED_LIBRARIES += SDL2_ttf
LOCAL_SHARED_LIBRARIES += SDL2_atk
LOCAL_SHARED_LIBRARIES += SDL2_app

LOCAL_STATIC_LIBRARIES := CUGL

LOCAL_LDLIBS := -lGLESv1_CM -lGLESv2 -lGLESv3 -lOpenSLES -llog -landroid
include $(BUILD_SHARED_LIBRARY)
