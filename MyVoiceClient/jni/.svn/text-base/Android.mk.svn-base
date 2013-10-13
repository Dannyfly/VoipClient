# Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
#
# Use of this source code is governed by a BSD-style license
# that can be found in the LICENSE file in the root of the source
# tree. An additional intellectual property rights grant can be found
# in the file PATENTS.  All contributing project authors may
# be found in the AUTHORS file in the root of the source tree.

MY_WEBRTC_ROOT_PATH := $(call my-dir)/../../libjingle/third_party

#common
include $(MY_WEBRTC_ROOT_PATH)/webrtc/modules/utility/source/Android.mk
include $(MY_WEBRTC_ROOT_PATH)/webrtc/system_wrappers/source/Android.mk

########################################################################
##								Video
########################################################################
include $(MY_WEBRTC_ROOT_PATH)/android-webrtc.mk
ifeq ($(HAS_COMPILE_VIDEO), 1)
#video common
include $(MY_WEBRTC_ROOT_PATH)/webrtc/video_engine/Android.mk
include $(MY_WEBRTC_ROOT_PATH)/webrtc/common_video/Android.mk
include $(MY_WEBRTC_ROOT_PATH)/webrtc/modules/video_processing/main/source/Android.mk
#video capture
include $(MY_WEBRTC_ROOT_PATH)/webrtc/modules/video_capture/Android.mk
#video render
include $(MY_WEBRTC_ROOT_PATH)/webrtc/modules/video_render/Android.mk
#video coding
include $(MY_WEBRTC_ROOT_PATH)/webrtc/modules/video_coding/main/source/Android.mk
include $(MY_WEBRTC_ROOT_PATH)/webrtc/modules/video_coding/utility/Android.mk
include $(MY_WEBRTC_ROOT_PATH)/webrtc/modules/video_coding/codecs/i420/main/source/Android.mk
#include $(MY_WEBRTC_ROOT_PATH)/webrtc/modules/video_coding/codecs/vp8/Android.mk
include $(MY_WEBRTC_ROOT_PATH)/libyuv/Android.mk
#include $(MY_WEBRTC_ROOT_PATH)/libvpx/source/libvpx/build/make/Android.mk
endif


########################################################################
##								Audio
########################################################################
#audio common
include $(MY_WEBRTC_ROOT_PATH)/webrtc/voice_engine/Android.mk
include $(MY_WEBRTC_ROOT_PATH)/webrtc/common_audio/resampler/Android.mk
include $(MY_WEBRTC_ROOT_PATH)/webrtc/common_audio/signal_processing/Android.mk
include $(MY_WEBRTC_ROOT_PATH)/webrtc/common_audio/vad/Android.mk

#audio coding
include $(MY_WEBRTC_ROOT_PATH)/webrtc/modules/audio_coding/neteq/Android.mk
include $(MY_WEBRTC_ROOT_PATH)/webrtc/modules/audio_coding/codecs/cng/Android.mk
include $(MY_WEBRTC_ROOT_PATH)/webrtc/modules/audio_coding/codecs/g711/Android.mk
include $(MY_WEBRTC_ROOT_PATH)/webrtc/modules/audio_coding/codecs/g722/Android.mk
include $(MY_WEBRTC_ROOT_PATH)/webrtc/modules/audio_coding/codecs/pcm16b/Android.mk
include $(MY_WEBRTC_ROOT_PATH)/webrtc/modules/audio_coding/codecs/ilbc/Android.mk
include $(MY_WEBRTC_ROOT_PATH)/webrtc/modules/audio_coding/codecs/iSAC/fix/source/Android.mk
include $(MY_WEBRTC_ROOT_PATH)/webrtc/modules/audio_coding/codecs/iSAC/main/source/Android.mk
include $(MY_WEBRTC_ROOT_PATH)/webrtc/modules/audio_coding/codecs/silk/Android.mk
include $(MY_WEBRTC_ROOT_PATH)/webrtc/modules/audio_coding/main/source/Android.mk

#audio processing
include $(MY_WEBRTC_ROOT_PATH)/webrtc/modules/audio_processing/aec/Android.mk
include $(MY_WEBRTC_ROOT_PATH)/webrtc/modules/audio_processing/aecm/Android.mk
include $(MY_WEBRTC_ROOT_PATH)/webrtc/modules/audio_processing/agc/Android.mk
include $(MY_WEBRTC_ROOT_PATH)/webrtc/modules/audio_processing/Android.mk
include $(MY_WEBRTC_ROOT_PATH)/webrtc/modules/audio_processing/ns/Android.mk
include $(MY_WEBRTC_ROOT_PATH)/webrtc/modules/audio_processing/utility/Android.mk
 
#audio mixer
include $(MY_WEBRTC_ROOT_PATH)/webrtc/modules/audio_conference_mixer/source/Android.mk

#audio device
include $(MY_WEBRTC_ROOT_PATH)/webrtc/modules/audio_device/Android.mk


########################################################################
##								Transport
########################################################################
#transport
include $(MY_WEBRTC_ROOT_PATH)/webrtc/modules/rtp_rtcp/source/Android.mk
include $(MY_WEBRTC_ROOT_PATH)/webrtc/modules/udp_transport/source/Android.mk
include $(MY_WEBRTC_ROOT_PATH)/webrtc/modules/bitrate_controller/Android.mk
include $(MY_WEBRTC_ROOT_PATH)/webrtc/modules/remote_bitrate_estimator/Android.mk
include $(MY_WEBRTC_ROOT_PATH)/webrtc/modules/pacing/Android.mk


########################################################################
##								MediaFile
########################################################################
#media file
include $(MY_WEBRTC_ROOT_PATH)/webrtc/modules/media_file/source/Android.mk

#test
#include $(MY_WEBRTC_ROOT_PATH)/webrtc/test/Android.mk


# build .so
LOCAL_PATH := $(PWD)

include $(CLEAR_VARS)

include $(MY_WEBRTC_ROOT_PATH)/android-webrtc.mk

LOCAL_CPP_EXTENSION := .cc .cpp
LOCAL_SRC_FILES := \
	android_test.cc \
	lost_estimate.cc \

LOCAL_C_INCLUDES := $(MY_WEBRTC_ROOT_PATH)/webrtc/modules/video_render/include \
	$(MY_WEBRTC_ROOT_PATH)/webrtc \
	$(MY_WEBRTC_ROOT_PATH)/webrtc/voice_engine/test/auto_test \
	$(MY_WEBRTC_ROOT_PATH)/webrtc/voice_engine/include \
	$(MY_WEBRTC_ROOT_PATH) \
	$(MY_WEBRTC_ROOT_PATH)/webrtc/modules/utility/interface \
	$(MY_WEBRTC_ROOT_PATH)/webrtc/modules/interface \
	$(MY_WEBRTC_ROOT_PATH)/webrtc/system_wrappers/interface \
	$(MY_WEBRTC_ROOT_PATH)/webrtc/modules/video_render/android \
	$(MY_WEBRTC_ROOT_PATH)/webrtc/modules/video_capture/include \
	$(MY_WEBRTC_ROOT_PATH)/webrtc/common_video/interface \
	$(MY_WEBRTC_ROOT_PATH)/webrtc/common_video/libyuv/include\
	$(MY_WEBRTC_ROOT_PATH)/webrtc/modules/video_coding/main/source \


LOCAL_ARM_MODE := arm
LOCAL_MODULE := libwebrtcx
LOCAL_MODULE_TAGS := optional
LOCAL_LDLIBS := -llog -lgcc -lGLESv2 -lOpenSLES -Llibs -lsilk
LOCAL_CFLAGS += $(MY_WEBRTC_COMMON_DEFS)

LOCAL_WHOLE_STATIC_LIBRARIES := \
	libwebrtc_utility \
	libwebrtc_system_wrappers

ifeq ($(HAS_COMPILE_VIDEO), 1)
LOCAL_WHOLE_STATIC_LIBRARIES += \
	libcommon_video \
	libwebrtc_video_processing \
	libwebrtc_video_render \
	libwebrtc_video_capture \
	libwebrtc_video_coding \
	libvideo_coding_utility \
	libwebrtc_i420 \
	libyuv_static \
	libwebrtc_vie_core 
endif

LOCAL_WHOLE_STATIC_LIBRARIES += \
	libwebrtc_voe_core \
	libwebrtc_spl \
	libwebrtc_resampler \
	libwebrtc_vad \
	libwebrtc_neteq \
	libwebrtc_cng \
	libwebrtc_g711 \
	libwebrtc_g722 \
	libwebrtc_pcm16b \
	libwebrtc_ilbc \
	libwebrtc_isacfix \
	libwebrtc_isac \
	libwebrtc_silk \
	libwebrtc_audio_coding \
	libwebrtc_aec \
	libwebrtc_aecm \
	libwebrtc_agc \
	libwebrtc_ns \
	libwebrtc_apm \
	libwebrtc_apm_utility \
	libwebrtc_audio_conference_mixer \
	libwebrtc_audio_device \

LOCAL_WHOLE_STATIC_LIBRARIES += \
	libwebrtc_rtp_rtcp \
	libwebrtc_bitrate_controller \
	libwebrtc_remote_bitrate_estimator \
	libwebrtc_pacing \
	libwebrtc_media_file \
	libwebrtc_udp_transport

# libwebrtc_test_support \


# Add Neon libraries.
ifeq ($(WEBRTC_BUILD_NEON_LIBS),true)
LOCAL_WHOLE_STATIC_LIBRARIES += \
    libwebrtc_aecm_neon \
    libwebrtc_ns_neon \
    libwebrtc_spl_neon \
	libwebrtc_isacfix_neon
endif

#LOCAL_STATIC_LIBRARIES :=  libprotobuf-cpp-2.3.0-lite

#LOCAL_ALLOW_UNDEFINED_SYMBOLS := true

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    libdl \
    libstlport \
	libvpx

LOCAL_PRELINK_MODULE := false

ifndef NDK_ROOT
include external/stlport/libstlport.mk
endif
#include $(BUILD_STATIC_LIBRARY)
include $(BUILD_SHARED_LIBRARY)
