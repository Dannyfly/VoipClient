#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

//#ifdef OPEN_LOG_TRACE
#ifdef ANDROID
#include <android/log.h>
#define THIS_FILE " android_test.cc "
#define MYLOG(...) __android_log_print(ANDROID_LOG_DEBUG, THIS_FILE, __VA_ARGS__)
#endif
//#endif
 
#include "org_webrtc_voiceengine_test_AndroidTest.h"

#include <string>
using namespace std;

#include "thread_wrapper.h"

#include "voe_base.h"
#include "voe_codec.h"
#include "voe_file.h"
#include "voe_network.h"
#include "voe_audio_processing.h"
#include "voe_volume_control.h"
#include "voe_hardware.h"
#include "voe_rtp_rtcp.h"
#include "voe_encryption.h"
#include "voe_test_interface.h"


#define _FATAL_ 0   //"ANDROID_LOG_TATAL"
#define THIS_TAG "android_test.cc"


int g_bitRate = 0;
int g_lossPencentage = 0;


//#define USE_SRTP
//#define INIT_FROM_THREAD
//#define START_CALL_FROM_THREAD
 using namespace webrtc;

#define WEBRTC_LOG_TAG "android_test.cc" // As in WEBRTC Native...
#define VALIDATE_BASE_POINTER \
    if (!veData1.base) \
    { \
        __android_log_write(ANDROID_LOG_ERROR, WEBRTC_LOG_TAG, \
                            "Base pointer doesn't exist"); \
        return -1; \
    }
#define VALIDATE_CODEC_POINTER \
    if (!veData1.codec) \
    { \
        __android_log_write(ANDROID_LOG_ERROR, WEBRTC_LOG_TAG, \
                            "Codec pointer doesn't exist"); \
        return -1; \
    }
#define VALIDATE_FILE_POINTER \
    if (!veData1.file) \
    { \
        __android_log_write(ANDROID_LOG_ERROR, WEBRTC_LOG_TAG, \
                            "File pointer doesn't exist"); \
        return -1; \
    }
#define VALIDATE_NETWORK_POINTER \
    if (!veData1.netw) \
    { \
        __android_log_write(ANDROID_LOG_ERROR, WEBRTC_LOG_TAG, \
                            "Network pointer doesn't exist"); \
        return -1; \
    }
#define VALIDATE_APM_POINTER \
    if (!veData1.apm) \
    { \
        __android_log_write(ANDROID_LOG_ERROR, WEBRTC_LOG_TAG, \
                            "Apm pointer doesn't exist"); \
        return -1; \
    }
#define VALIDATE_VOLUME_POINTER \
    if (!veData1.volume) \
    { \
        __android_log_write(ANDROID_LOG_ERROR, WEBRTC_LOG_TAG, \
                            "Volume pointer doesn't exist"); \
        return -1; \
    }
#define VALIDATE_HARDWARE_POINTER \
    if (!veData1.hardware) \
    { \
        __android_log_write(ANDROID_LOG_ERROR, WEBRTC_LOG_TAG, \
                            "Hardware pointer doesn't exist"); \
        return -1; \
    }
#define VALIDATE_RTP_RTCP_POINTER \
    if (!veData1.rtp_rtcp) \
    { \
        __android_log_write(ANDROID_LOG_ERROR, WEBRTC_LOG_TAG, \
                            "RTP / RTCP pointer doesn't exist"); \
        return -1; \
    }
#define VALIDATE_ENCRYPT_POINTER \
    if (!veData1.encrypt) \
    { \
        __android_log_write(ANDROID_LOG_ERROR, WEBRTC_LOG_TAG, \
                            "Encrypt pointer doesn't exist"); \
        return -1; \
    }



class my_transportation;

// VoiceEngine data struct
typedef struct
{
    // VoiceEngine
    VoiceEngine* ve;  //
    // Sub-APIs
    VoEBase* base;
    VoECodec* codec;
    VoEFile* file;
    VoENetwork* netw;
    VoEAudioProcessing* apm;
    VoEVolumeControl* volume;
    VoEHardware* hardware;
    VoERTP_RTCP* rtp_rtcp;
    VoEEncryption* encrypt;
    // Other
    my_transportation* extTrans;
    JavaVM* jvm;

    // VoVoENetEqStats* neteqstatus;    // jitter buffer的大小

} VoiceEngineData;


#if 0
// my_transportation is used when useExtTrans is enabled
class my_transportation : public Transport
{
 public:
  my_transportation(VoENetwork * network) :
      netw(network) {
  }

  int SendPacket(int channel,const void *data,int len);
  int SendRTCPPacket(int channel, const void *data, int len);
 private:
  VoENetwork * netw;
};

int my_transportation::SendPacket(int channel,const void *data,int len)
{
  netw->ReceivedRTPPacket(channel, data, len);
  return len;
}

int my_transportation::SendRTCPPacket(int channel, const void *data, int len)
{
  netw->ReceivedRTCPPacket(channel, data, len);
  return len;
}
#endif

//Global variables visible in this file
static VoiceEngineData veData1;
static VoiceEngineData veData2;

// "Local" functions (i.e. not Java accessible)
static bool GetSubApis(VoiceEngineData &veData);
static bool ReleaseSubApis(VoiceEngineData &veData);


//////////////////////////////////////////////////////////////////
// General functions
//////////////////////////////////////////////////////////////////

/////////////////////////////////////////////
//              JNI_OnLoad
//
jint JNI_OnLoad(JavaVM* vm, void* /*reserved*/)
{
    if (!vm)
    {
        __android_log_write(ANDROID_LOG_ERROR, WEBRTC_LOG_TAG,
                            "JNI_OnLoad did not receive a valid VM pointer");
        return -1;
    }

    // Get JNI
    JNIEnv* env;
    if (JNI_OK != vm->GetEnv(reinterpret_cast<void**> (&env),
                             JNI_VERSION_1_4))
    {
        __android_log_write(ANDROID_LOG_ERROR, WEBRTC_LOG_TAG,
                            "JNI_OnLoad could not get JNI env");
        return -1;
    }

    // Get class to register the native functions with
    // jclass regClass = env->FindClass("webrtc/android/AndroidTest");
    // if (!regClass) {
    // return -1; // Exception thrown
    // }

    // Register native functions
    // JNINativeMethod methods[1];
    // methods[0].name = NULL;
    // methods[0].signature = NULL;
    // methods[0].fnPtr = NULL;
    // if (JNI_OK != env->RegisterNatives(regClass, methods, 1))
    // {
    // return -1;
    // }

    // Init VoiceEngine data
    memset(&veData1, 0, sizeof(veData1));
    memset(&veData2, 0, sizeof(veData2));

    // Store the JVM
    veData1.jvm = vm;
    veData2.jvm = vm;

    return JNI_VERSION_1_4;
}

/////////////////////////////////////////////
// Native initialization
//
JNIEXPORT jboolean JNICALL
Java_org_webrtc_voiceengine_test_AndroidTest_NativeInit(
        JNIEnv * env,
        jclass)
{
    // Look up and cache any interesting class, field and method IDs for
    // any used java class here

    return true;
}


// 设置编码比特率和丢包率
JNIEXPORT jboolean JNICALL Java_org_webrtc_voiceengine_test_AndroidTest_SetParam(
        JNIEnv *env,
        jobject context,
        jint bitRate, 
        jint lossPencentage)
{
    MYLOG("Info: set param bitRate(%d)  lossPencentage(%d)", bitRate, lossPencentage);
    g_bitRate = bitRate;
    g_lossPencentage = lossPencentage;
    MYLOG("Info: set param g_bitRate(%d)  g_lossPencentage(%d)", g_bitRate, g_lossPencentage);
    return true;
}



//////////////////////////////////////////////////////////////////
// VoiceEngine API wrapper functions
//////////////////////////////////////////////////////////////////

/////////////////////////////////////////////
// Create VoiceEngine instance
//
JNIEXPORT jboolean JNICALL Java_org_webrtc_voiceengine_test_AndroidTest_Create(
        JNIEnv *env,
        jobject context)
{
    MYLOG("enter Java_org_webrtc_voiceengine_test_AndroidTest_Create %p  %p ... ", env, context);
说
    const char* str = "Java_org_webrtc_voiceengine_test_AndroidTest_Create";
    __android_log_print(ANDROID_LOG_ERROR, THIS_FILE, "%s%s", "Enter ", str);
    
    // Check if already created
    if (veData1.ve)
    {
        __android_log_write(ANDROID_LOG_ERROR, WEBRTC_LOG_TAG,
                            " ####### VoE already created ####### ");
        return false;
    }

    // Set instance independent Java objects
    VoiceEngine::SetAndroidObjects(veData1.jvm, env, context);

#ifdef START_CALL_FROM_THREAD
    threadTest.RunTest();
#else
    // Create
    veData1.ve = VoiceEngine::Create();
    if (!veData1.ve)
    {
        __android_log_write(ANDROID_LOG_ERROR, WEBRTC_LOG_TAG,
                            "Create VoE failed");
        return false;
    }

    // Get sub-APIs
    if (!GetSubApis(veData1))
    {
        // If not OK, release all sub-APIs and delete VoE
        ReleaseSubApis(veData1);
        if (!VoiceEngine::Delete(veData1.ve))
        {
            __android_log_write(ANDROID_LOG_ERROR, WEBRTC_LOG_TAG,
                                "Delete VoE failed");
        }
        return false;
    }
#endif

    return true;
}

/////////////////////////////////////////////
// Delete VoiceEngine instance
//
JNIEXPORT jboolean JNICALL Java_org_webrtc_voiceengine_test_AndroidTest_Delete(
        JNIEnv *,
        jobject)
{
#ifdef START_CALL_FROM_THREAD
    threadTest.CloseTest();
#else
    // Check if exists
    if (!veData1.ve)
    {
        __android_log_write(ANDROID_LOG_ERROR, WEBRTC_LOG_TAG,
                            "VoE does not exist");
        return false;
    }

    // Release sub-APIs
    ReleaseSubApis(veData1);

    // Delete
    if (!VoiceEngine::Delete(veData1.ve))
    {
        __android_log_write(ANDROID_LOG_ERROR, WEBRTC_LOG_TAG,
                            "Delete VoE failed");
        return false;
    }

    veData1.ve = NULL;
#endif

    // Clear instance independent Java objects
    VoiceEngine::SetAndroidObjects(NULL, NULL, NULL);

    return true;
}

/////////////////////////////////////////////
// [Base] Initialize VoiceEngine
//
JNIEXPORT jint JNICALL Java_org_webrtc_voiceengine_test_AndroidTest_Init(
        JNIEnv *,
        jobject,
        jboolean enableTrace = NULL,
        jboolean useExtTrans = NULL)
{
    VALIDATE_BASE_POINTER;

    if (enableTrace)
    {
        if (0 != VoiceEngine::SetTraceFile("/sdcard/trace.txt"))
        {
            __android_log_write(ANDROID_LOG_ERROR, WEBRTC_LOG_TAG,
                                "Could not enable trace");
        }
        if (0 != VoiceEngine::SetTraceFilter(kTraceAll))
        {
            __android_log_write(ANDROID_LOG_WARN, WEBRTC_LOG_TAG,
                                "Could not set trace filter");
        }
    }

    if (useExtTrans)
    {
        VALIDATE_NETWORK_POINTER;
        veData1.extTrans = new my_transportation(veData1.netw);
    }

    int retVal = 0;
#ifdef INIT_FROM_THREAD
    threadTest.RunTest();
    usleep(200000);
#else
    MYLOG(" #### VoEBase->Init() ####");
    retVal = veData1.base->Init();
#endif
    return retVal;
}

/////////////////////////////////////////////
// [Base] Terminate VoiceEngine
//
JNIEXPORT jint JNICALL Java_org_webrtc_voiceengine_test_AndroidTest_Terminate(
        JNIEnv *,
        jobject)
{
    VALIDATE_BASE_POINTER;

    jint retVal = veData1.base->Terminate();

    delete veData1.extTrans;
    veData1.extTrans = NULL;

    return retVal;
}

/////////////////////////////////////////////
// [Base] Create channel
//
JNIEXPORT jint JNICALL
Java_org_webrtc_voiceengine_test_AndroidTest_CreateChannel(
        JNIEnv *,
        jobject)
{
    MYLOG(" #### VoEBase->CreateChannel() ####");
    VALIDATE_BASE_POINTER;
    jint channel = veData1.base->CreateChannel();

    if (veData1.extTrans)
    {
        VALIDATE_NETWORK_POINTER;
        __android_log_print(ANDROID_LOG_DEBUG, WEBRTC_LOG_TAG,
                            "Enabling external transport on channel %d",
                            channel);
        if (veData1.netw->RegisterExternalTransport(channel, *veData1.extTrans)
                < 0)
        {
            __android_log_write(ANDROID_LOG_ERROR, WEBRTC_LOG_TAG,
                                "Could not set external transport");
            return -1;
        }
    }

    return channel;
}

/////////////////////////////////////////////
// [Base] Delete channel
//
JNIEXPORT jint JNICALL
Java_org_webrtc_voiceengine_test_AndroidTest_DeleteChannel(
        JNIEnv *,
        jobject,
        jint channel)
{
    VALIDATE_BASE_POINTER;
    return veData1.base->DeleteChannel(channel);
}

/////////////////////////////////////////////
// [Base] SetLocalReceiver
JNIEXPORT jint JNICALL
Java_org_webrtc_voiceengine_test_AndroidTest_SetLocalReceiver(
        JNIEnv *,
        jobject,
        jint channel,
        jint port)
{
    VALIDATE_BASE_POINTER;
    MYLOG(" #### VoEBase->SetLocalReceiver() ####");
    return veData1.base->SetLocalReceiver(channel, port);
}

/////////////////////////////////////////////
// [Base] SetSendDestination
//
JNIEXPORT jint JNICALL
Java_org_webrtc_voiceengine_test_AndroidTest_SetSendDestination(
        JNIEnv *env,
        jobject,
        jint channel,
        jint port,
        jstring ipaddr)
{
    VALIDATE_BASE_POINTER;

    const char* ipaddrNative = env->GetStringUTFChars(ipaddr, NULL);
    if (!ipaddrNative)
    {
        __android_log_write(ANDROID_LOG_ERROR, WEBRTC_LOG_TAG,
                            "Could not get UTF string");
        return -1;
    }

    MYLOG(" #### VoEBase->SetSendDestination() ####");
    MYLOG(" ######## channel(%d) port(%d) ipaddrNative(%s) ######## ", channel, port, ipaddrNative);
    jint retVal = veData1.base->SetSendDestination(channel, port, ipaddrNative);

    env->ReleaseStringUTFChars(ipaddr, ipaddrNative);

    return retVal;
}

/////////////////////////////////////////////
// [Base] StartListen
//
JNIEXPORT jint JNICALL Java_org_webrtc_voiceengine_test_AndroidTest_StartListen(
        JNIEnv *,
        jobject,
        jint channel)
{
#ifdef USE_SRTP
    VALIDATE_ENCRYPT_POINTER;
    bool useForRTCP = false;

    if (veData1.encrypt->EnableSRTPReceive(
                    channel,CIPHER_AES_128_COUNTER_MODE,30,AUTH_HMAC_SHA1,
                    16,4, ENCRYPTION_AND_AUTHENTICATION,
                    (unsigned char*)nikkey, useForRTCP) != 0)
    {
        __android_log_write(ANDROID_LOG_ERROR, WEBRTC_LOG_TAG,
                "Failed to enable SRTP receive");
        return -1;
    }
#endif

    VALIDATE_BASE_POINTER;
    MYLOG(" #### VoEBase->StartReceive() ####");
    int retVal = veData1.base->StartReceive(channel);

    return retVal;
}

/////////////////////////////////////////////
// [Base] Start playout
//
JNIEXPORT jint JNICALL
Java_org_webrtc_voiceengine_test_AndroidTest_StartPlayout(
        JNIEnv *,
        jobject,
        jint channel)
{
    VALIDATE_BASE_POINTER;
    MYLOG(" #### VoEBase->StartPlayout() ####");
    int retVal = veData1.base->StartPlayout(channel);

    return retVal;
}

/////////////////////////////////////////////
// [Base] Start send
//
JNIEXPORT jint JNICALL Java_org_webrtc_voiceengine_test_AndroidTest_StartSend(
        JNIEnv *,
        jobject,
        jint channel)
{

#ifdef USE_SRTP
    VALIDATE_ENCRYPT_POINTER;
    bool useForRTCP = false;
    if (veData1.encrypt->EnableSRTPSend(
                    channel,CIPHER_AES_128_COUNTER_MODE,30,AUTH_HMAC_SHA1,
                    16,4, ENCRYPTION_AND_AUTHENTICATION,
                    (unsigned char*)nikkey, useForRTCP) != 0)
    {
        __android_log_write(ANDROID_LOG_ERROR, WEBRTC_LOG_TAG,
                "Failed to enable SRTP send");
        return -1;
    }
#endif

    VALIDATE_BASE_POINTER;
    MYLOG(" #### VoEBase->StartSend() ####");
    int retVal = veData1.base->StartSend(channel);

    return retVal;
}

/////////////////////////////////////////////
// [Base] Stop listen
//
JNIEXPORT jint JNICALL Java_org_webrtc_voiceengine_test_AndroidTest_StopListen(
        JNIEnv *,
        jobject,
        jint channel)
{
#ifdef USE_SRTP
    VALIDATE_ENCRYPT_POINTER;
    if (veData1.encrypt->DisableSRTPReceive(channel) != 0)
    {
        __android_log_write(ANDROID_LOG_ERROR, WEBRTC_LOG_TAG,
                "Failed to disable SRTP receive");
        return -1;
    }
#endif

    VALIDATE_BASE_POINTER;
    return veData1.base->StopReceive(channel);
}

/////////////////////////////////////////////
// [Base] Stop playout
//
JNIEXPORT jint JNICALL Java_org_webrtc_voiceengine_test_AndroidTest_StopPlayout(
        JNIEnv *,
        jobject,
        jint channel)
{
    VALIDATE_BASE_POINTER;
    return veData1.base->StopPlayout(channel);
}

/////////////////////////////////////////////
// [Base] Stop send
//
JNIEXPORT jint JNICALL Java_org_webrtc_voiceengine_test_AndroidTest_StopSend(
        JNIEnv *,
        jobject,
        jint channel)
{
#ifdef USE_SRTP
    VALIDATE_ENCRYPT_POINTER;
    if (veData1.encrypt->DisableSRTPSend(channel) != 0)
    {
        __android_log_write(ANDROID_LOG_ERROR, WEBRTC_LOG_TAG,
                "Failed to disable SRTP send");
        return -1;
    }
#endif

    VALIDATE_BASE_POINTER;
    return veData1.base->StopSend(channel);
}

/////////////////////////////////////////////
// [codec] Number of codecs
//
JNIEXPORT jint JNICALL Java_org_webrtc_voiceengine_test_AndroidTest_NumOfCodecs(
        JNIEnv *,
        jobject)
{
    VALIDATE_CODEC_POINTER;
    MYLOG(" #### VoECodec->NumOfCodecs() ####");
    return veData1.codec->NumOfCodecs();
}

/////////////////////////////////////////////
// get all audio codec
//
JNIEXPORT jobjectArray JNICALL Java_org_webrtc_voiceengine_test_AndroidTest_GetAllCodecs(
        JNIEnv *env,
        jobject)
{
  if (!veData1.codec) {
    MYLOG(" ######### Codec pointer doesn't exist ######### ");
    return NULL;
  }

  jobjectArray ret;
  int i;
  int num = veData1.codec->NumOfCodecs();
  char info[256];

  ret = (jobjectArray)env->NewObjectArray(num,
                                          env->FindClass("java/lang/String"),
                                          env->NewStringUTF(""));

  for(i = 0; i < num; i++) {
    webrtc::CodecInst codecToList;
    veData1.codec->GetCodec(i, codecToList);
    int written = snprintf(info, sizeof(info),
                           "%s type:%d freq:%d pac:%d ch:%d rate:%d",
                           codecToList.plname, codecToList.pltype,
                           codecToList.plfreq, codecToList.pacsize,
                           codecToList.channels, codecToList.rate);
    assert(written >= 0 && written < static_cast<int>(sizeof(info)));
    MYLOG(" ######## VoiceEgnine Codec[%d] %s ######## ", i, info);
    env->SetObjectArrayElement(ret, i, env->NewStringUTF( info ));
  }

  return ret;
}

/////////////////////////////////////////////
// Enable High Pass Filter
//
JNIEXPORT jint JNICALL Java_org_webrtc_voiceengine_test_AndroidTest_SetHighPassFilter
  (JNIEnv *env, jobject, jboolean enable)
{
  VALIDATE_APM_POINTER;
  MYLOG("###### Enable High Pass Filter %d ######", enable);
  return veData1.apm->EnableHighPassFilter(enable);
}


JNIEXPORT jint JNICALL Java_org_webrtc_voiceengine_test_AndroidTest_SetAecmMode
  (JNIEnv *env, jobject, jint mode, jboolean enable)
{
  VALIDATE_APM_POINTER;
  MYLOG("###### Set Aecm Mode %d  %d ######", mode, enable);

  AecmModes aecmMode;
  switch(mode)
  {
    case 0:
        aecmMode = kAecmQuietEarpieceOrHeadset;
        break;
    case 1:
        aecmMode = kAecmEarpiece;
        break;
    case 2:
        aecmMode = kAecmLoudEarpiece;
        break;
    case 3:
        aecmMode = kAecmSpeakerphone;
        break;
    case 4:
        aecmMode = kAecmLoudSpeakerphone;
        break;
    default:
        aecmMode = kAecmEarpiece;
        break;
  }
  return veData1.apm->SetAecmMode(aecmMode, enable);
}

JNIEXPORT jint JNICALL Java_org_webrtc_voiceengine_test_AndroidTest_SetDelayOffsetMs
  (JNIEnv *env, jobject, jint offset)
{
    VALIDATE_APM_POINTER;
    veData1.apm->SetDelayOffsetMs(offset);
    return 0;
}

/////////////////////////////////////////////
// [codec] Set send codec
//
JNIEXPORT jint JNICALL
Java_org_webrtc_voiceengine_test_AndroidTest_SetSendCodec(
        JNIEnv *,
        jobject,
        jint channel,
        jint index)
{
    VALIDATE_CODEC_POINTER;

    CodecInst codec;

    if (veData1.codec->GetCodec(index, codec) != 0)
    {
        __android_log_write(ANDROID_LOG_ERROR, WEBRTC_LOG_TAG,
                            "Failed to get codec");
        return -1;
    }

    MYLOG(" #### VoECodec->SetSendCodec() ####");
    MYLOG(" #### pltype(%d) ####",   codec.pltype);
    MYLOG(" #### plname(%s) ####",   codec.plname);
    MYLOG(" #### plfreq(%d) ####",   codec.plfreq);
    MYLOG(" #### pacsize(%d) ####",  codec.pacsize);
    MYLOG(" #### channels(%d) ####", codec.channels);
    MYLOG(" #### rate(%d) ####",     codec.rate);

    if (g_bitRate != 0)
    {
         codec.rate = g_bitRate;
         MYLOG(" #### ~~~~~ rate(%d) ####",     codec.rate);
     }

    return veData1.codec->SetSendCodec(channel, codec);
}

/////////////////////////////////////////////
// [codec] Set VAD status
//
JNIEXPORT jint JNICALL
Java_org_webrtc_voiceengine_test_AndroidTest_SetVADStatus(
        JNIEnv *,
        jobject,
        jint channel,
        jboolean enable,
        jint mode)
{
    VALIDATE_CODEC_POINTER;

    VadModes VADmode = kVadConventional;

    switch (mode)
    {
        case 0:
            break; // already set
        case 1:
            VADmode = kVadAggressiveLow;
            break;
        case 2:
            VADmode = kVadAggressiveMid;
            break;
        case 3:
            VADmode = kVadAggressiveHigh;
            break;
        default:
            VADmode = (VadModes) 17; // force error
            break;
    }

    return veData1.codec->SetVADStatus(channel, enable, VADmode);
}

/////////////////////////////////////////////
// [apm] SetNSStatus
//
JNIEXPORT jint JNICALL Java_org_webrtc_voiceengine_test_AndroidTest_SetNSStatus(
        JNIEnv *,
        jobject,
        jboolean enable,
        jint mode)
{
    VALIDATE_APM_POINTER;

    NsModes NSmode = kNsDefault;

    switch (mode)
    {
        case 0:
            NSmode = kNsUnchanged;
            break;
        case 1:
            break; // already set
        case 2:
            NSmode = kNsConference;
            break;
        case 3:
            NSmode = kNsLowSuppression;
            break;
        case 4:
            NSmode = kNsModerateSuppression;
            break;
        case 5:
            NSmode = kNsHighSuppression;
            break;
        case 6:
            NSmode = kNsVeryHighSuppression;
            break;
        default:
            NSmode = (NsModes) 17; // force error
            break;
    }

    return veData1.apm->SetNsStatus(enable, NSmode);
}

/////////////////////////////////////////////
// [apm] SetAGCStatus
//
JNIEXPORT jint JNICALL
Java_org_webrtc_voiceengine_test_AndroidTest_SetAGCStatus(
        JNIEnv *,
        jobject,
        jboolean enable,
        jint mode)
{
    VALIDATE_APM_POINTER;

    AgcModes AGCmode = kAgcDefault;

    switch (mode)
    {
        case 0:
            AGCmode = kAgcUnchanged;
            break;
        case 1:
            break; // already set
        case 2:
            AGCmode = kAgcAdaptiveAnalog;
            break;
        case 3:
            AGCmode = kAgcAdaptiveDigital;
            break;
        case 4:
            AGCmode = kAgcFixedDigital;
            break;
        default:
            AGCmode = (AgcModes) 17; // force error
            break;
    }

    return veData1.apm->SetAgcStatus(enable, AGCmode);
}

/////////////////////////////////////////////
// [apm] SetECStatus
//
JNIEXPORT jint JNICALL Java_org_webrtc_voiceengine_test_AndroidTest_SetECStatus(
        JNIEnv *,
        jobject,
        jboolean enable,
        jint mode)
{
    VALIDATE_APM_POINTER;

    EcModes ECmode = kEcDefault;

    switch (mode)
    {
        case 0:
            ECmode = kEcDefault;
            break;
        case 1:
            break; // already set
        case 2:
            ECmode = kEcConference;
            break;
        case 3:
            ECmode = kEcAec;
            break;
        case 4:
            ECmode = kEcAecm;
            break;
        default:
            ECmode = (EcModes) 17; // force error
            break;
    }

    return veData1.apm->SetEcStatus(enable, ECmode);
}

/////////////////////////////////////////////
// [File] Start play file locally
//
JNIEXPORT jint JNICALL
Java_org_webrtc_voiceengine_test_AndroidTest_StartPlayingFileLocally(
        JNIEnv * env,
        jobject,
        jint channel,
        jstring fileName,
        jboolean loop)
{
    VALIDATE_FILE_POINTER;

    const char* fileNameNative = env->GetStringUTFChars(fileName, NULL);
    if (!fileNameNative)
    {
        __android_log_write(ANDROID_LOG_ERROR, WEBRTC_LOG_TAG,
                            "Could not get UTF string");
        return -1;
    }

    jint retVal = veData1.file->StartPlayingFileLocally(channel,
                                                        fileNameNative, loop);

    env->ReleaseStringUTFChars(fileName, fileNameNative);

    return retVal;
}

/////////////////////////////////////////////
// [File] Stop play file locally
//
JNIEXPORT jint JNICALL
Java_org_webrtc_voiceengine_test_AndroidTest_StopPlayingFileLocally(
        JNIEnv *,
        jobject,
        jint channel)
{
    VALIDATE_FILE_POINTER;
    return veData1.file->StopPlayingFileLocally(channel);
}

/*
 * Class:     org_webrtc_voiceengine_test_AndroidTest
 * Method:    StartRecordingPlayout
 * Signature: (ILjava/lang/String;Z)I
 */
JNIEXPORT jint JNICALL
Java_org_webrtc_voiceengine_test_AndroidTest_StartRecordingPlayout(
        JNIEnv * env,
        jobject,
        jint channel,
        jstring fileName,
        jboolean)
{
    VALIDATE_FILE_POINTER;

    const char* fileNameNative = env->GetStringUTFChars(fileName, NULL);
    if (!fileNameNative)
    {
        __android_log_write(ANDROID_LOG_ERROR, WEBRTC_LOG_TAG,
                            "Could not get UTF string");
        return -1;
    }

    jint retVal = veData1.file->StartRecordingPlayout(channel, fileNameNative,
                                                      0);

    env->ReleaseStringUTFChars(fileName, fileNameNative);

    return retVal;
}

/////////////////////////////////////////////
// [File] Stop Recording Playout
//
JNIEXPORT jint JNICALL
Java_org_webrtc_voiceengine_test_AndroidTest_StopRecordingPlayout(
        JNIEnv *,
        jobject,
        jint channel)
{
    VALIDATE_FILE_POINTER;
    return veData1.file->StopRecordingPlayout(channel);
}

/////////////////////////////////////////////
// [File] Start playing file as microphone
//
JNIEXPORT jint JNICALL
Java_org_webrtc_voiceengine_test_AndroidTest_StartPlayingFileAsMicrophone(
        JNIEnv *env,
        jobject,
        jint channel,
        jstring fileName,
        jboolean loop)
{
    VALIDATE_FILE_POINTER;

    const char* fileNameNative = env->GetStringUTFChars(fileName, NULL);
    if (!fileNameNative)
    {
        __android_log_write(ANDROID_LOG_ERROR, WEBRTC_LOG_TAG,
                            "Could not get UTF string");
        return -1;
    }

    jint retVal = veData1.file->StartPlayingFileAsMicrophone(channel,
                                                             fileNameNative,
                                                             loop);

    env->ReleaseStringUTFChars(fileName, fileNameNative);

    return retVal;
}

/////////////////////////////////////////////
// [File] Stop playing file as microphone
//
JNIEXPORT jint JNICALL
Java_org_webrtc_voiceengine_test_AndroidTest_StopPlayingFileAsMicrophone(
        JNIEnv *,
        jobject,
        jint channel)
{
    VALIDATE_FILE_POINTER;
    return veData1.file->StopPlayingFileAsMicrophone(channel);
}

/////////////////////////////////////////////
// [Volume] Set speaker volume
//
JNIEXPORT jint JNICALL
Java_org_webrtc_voiceengine_test_AndroidTest_SetSpeakerVolume(
        JNIEnv *,
        jobject,
        jint level)
{
    VALIDATE_VOLUME_POINTER;
    if (veData1.volume->SetSpeakerVolume(level) != 0)
    {
        return -1;
    }

    unsigned int storedVolume = 0;
    if (veData1.volume->GetSpeakerVolume(storedVolume) != 0)
    {
        return -1;
    }

    if (storedVolume != level)
    {
        return -1;
    }

    return 0;
}

/////////////////////////////////////////////
// [Hardware] Set loudspeaker status
//
JNIEXPORT jint JNICALL
Java_org_webrtc_voiceengine_test_AndroidTest_SetLoudspeakerStatus(
        JNIEnv *,
        jobject,
        jboolean enable)
{
    VALIDATE_HARDWARE_POINTER;
    MYLOG(" #### VoEHardware->SetLoudspeakerStatus() ####");
    if (veData1.hardware->SetLoudspeakerStatus(enable) != 0)
    {
        return -1;
    }


    return 0;
}

JNIEXPORT jint JNICALL Java_org_webrtc_voiceengine_test_AndroidTest_SetRxNsStatus(
        JNIEnv *,
        jobject,
        jint channel,
        jboolean enable,
        jint mode)
{
    VALIDATE_APM_POINTER;

   NsModes NSmode = kNsDefault;

    switch (mode)
    {
        case 0:
            NSmode = kNsUnchanged;
            break;
        case 1:
            break; // already set
        case 2:
            NSmode = kNsConference;
            break;
        case 3:
            NSmode = kNsLowSuppression;
            break;
        case 4:
            NSmode = kNsModerateSuppression;
            break;
        case 5:
            NSmode = kNsHighSuppression;
            break;
        case 6:
            NSmode = kNsVeryHighSuppression;
            break;
        default:
            NSmode = (NsModes) 17; // force error
            break;
    }

    return veData1.apm->SetRxNsStatus(channel,enable, NSmode);
}


JNIEXPORT jint JNICALL Java_org_webrtc_voiceengine_test_AndroidTest_SetRxAgcStatus(
        JNIEnv *,
        jobject ,
        jint channel,
        jboolean enable,
        jint mode)
{
    VALIDATE_APM_POINTER;

    AgcModes AGCmode = kAgcDefault;

    switch (mode)
    {
        case 0:
            AGCmode = kAgcUnchanged;
            break;
        case 1:
            break; // already set
        case 2:
            AGCmode = kAgcAdaptiveAnalog;
            break;
        case 3:
            AGCmode = kAgcAdaptiveDigital;
            break;
        case 4:
            AGCmode = kAgcFixedDigital;
            break;
        default:
            AGCmode = (AgcModes) 17; // force error
            break;
    }

    return veData1.apm->SetRxAgcStatus(channel,enable, AGCmode);
}


//////////////////////////////////////////////////////////////////
// "Local" functions (i.e. not Java accessible)
//////////////////////////////////////////////////////////////////

/////////////////////////////////////////////
// Get all sub-APIs
//
bool GetSubApis(VoiceEngineData &veData)
{
    bool getOK = true;

    // Base
    MYLOG(" veData.ve = %p ", veData.ve);
    veData.base = VoEBase::GetInterface(veData.ve);
    if (!veData.base)
    {
        __android_log_write(ANDROID_LOG_ERROR, WEBRTC_LOG_TAG,
                            "Get base sub-API failed");
        getOK = false;
    }
    MYLOG(" veData.base = %p ", veData.base);

    // Codec
    MYLOG(" veData.ve = %p ", veData.ve);
    veData.codec = VoECodec::GetInterface(veData.ve);
    if (!veData.codec)
    {
        __android_log_write(ANDROID_LOG_ERROR, WEBRTC_LOG_TAG,
                            "Get codec sub-API failed");
        getOK = false;
    }
    MYLOG(" veData.codec = %p ", veData.codec);

    // File
    veData.file = VoEFile::GetInterface(veData.ve);
    if (!veData.file)
    {
        __android_log_write(ANDROID_LOG_ERROR, WEBRTC_LOG_TAG,
                            "Get file sub-API failed");
        getOK = false;
    }

    // Network
    veData.netw = VoENetwork::GetInterface(veData.ve);
    if (!veData.netw)
    {
        __android_log_write(ANDROID_LOG_ERROR, WEBRTC_LOG_TAG,
                            "Get network sub-API failed");
        getOK = false;
    }

    // AudioProcessing module
    veData.apm = VoEAudioProcessing::GetInterface(veData.ve);
    if (!veData.apm)
    {
        __android_log_write(ANDROID_LOG_ERROR, WEBRTC_LOG_TAG,
                            "Get apm sub-API failed");
        getOK = false;
    }

    // Volume
    veData.volume = VoEVolumeControl::GetInterface(veData.ve);
    if (!veData.volume)
    {
        __android_log_write(ANDROID_LOG_ERROR, WEBRTC_LOG_TAG,
                            "Get volume sub-API failed");
        getOK = false;
    }

    // Hardware
    veData.hardware = VoEHardware::GetInterface(veData.ve);
    if (!veData.hardware)
    {
        __android_log_write(ANDROID_LOG_ERROR, WEBRTC_LOG_TAG,
                            "Get hardware sub-API failed");
        getOK = false;
    }

    // RTP / RTCP
    veData.rtp_rtcp = VoERTP_RTCP::GetInterface(veData.ve);
    if (!veData.rtp_rtcp)
    {
        __android_log_write(ANDROID_LOG_ERROR, WEBRTC_LOG_TAG,
                            "Get rtp_rtcp sub-API failed");
        getOK = false;
    }

    // Encrypt
    veData.encrypt = VoEEncryption::GetInterface(veData.ve);
    if (!veData.encrypt)
    {
        __android_log_write(ANDROID_LOG_ERROR, WEBRTC_LOG_TAG,
                            "Get encrypt sub-API failed");
        getOK = false;
    }

    return getOK;
}

/////////////////////////////////////////////
// Release all sub-APIs
//
bool ReleaseSubApis(VoiceEngineData &veData)
{
    bool releaseOK = true;

    // Base
    if (veData.base)
    {
        if (0 != veData.base->Release())
        {
            __android_log_write(ANDROID_LOG_ERROR, WEBRTC_LOG_TAG,
                                "Release base sub-API failed");
            releaseOK = false;
        }
        else
        {
            veData.base = NULL;
        }
    }

    // Codec
    if (veData.codec)
    {
        if (0 != veData.codec->Release())
        {
            __android_log_write(ANDROID_LOG_ERROR, WEBRTC_LOG_TAG,
                                "Release codec sub-API failed");
            releaseOK = false;
        }
        else
        {
            veData.codec = NULL;
        }
    }

    // File
    if (veData.file)
    {
        if (0 != veData.file->Release())
        {
            __android_log_write(ANDROID_LOG_ERROR, WEBRTC_LOG_TAG,
                                "Release file sub-API failed");
            releaseOK = false;
        }
        else
        {
            veData.file = NULL;
        }
    }

    // Network
    if (veData.netw)
    {
        if (0 != veData.netw->Release())
        {
            __android_log_write(ANDROID_LOG_ERROR, WEBRTC_LOG_TAG,
                                "Release network sub-API failed");
            releaseOK = false;
        }
        else
        {
            veData.netw = NULL;
        }
    }

    // apm
    if (veData.apm)
    {
        if (0 != veData.apm->Release())
        {
            __android_log_write(ANDROID_LOG_ERROR, WEBRTC_LOG_TAG,
                                "Release apm sub-API failed");
            releaseOK = false;
        }
        else
        {
            veData.apm = NULL;
        }
    }

    // Volume
    if (veData.volume)
    {
        if (0 != veData.volume->Release())
        {
            __android_log_write(ANDROID_LOG_ERROR, WEBRTC_LOG_TAG,
                                "Release volume sub-API failed");
            releaseOK = false;
        }
        else
        {
            veData.volume = NULL;
        }
    }

    // Hardware
    if (veData.hardware)
    {
        if (0 != veData.hardware->Release())
        {
            __android_log_write(ANDROID_LOG_ERROR, WEBRTC_LOG_TAG,
                                "Release hardware sub-API failed");
            releaseOK = false;
        }
        else
        {
            veData.hardware = NULL;
        }
    }

    // RTP RTCP
    if (veData.rtp_rtcp)
    {
        if (0 != veData.rtp_rtcp->Release())
        {
            __android_log_write(ANDROID_LOG_ERROR, WEBRTC_LOG_TAG,
                                "Release rtp_rtcp sub-API failed");
            releaseOK = false;
        }
        else
        {
            veData.rtp_rtcp = NULL;
        }
    }

    // Encrypt
    if (veData.encrypt)
    {
        if (0 != veData.encrypt->Release())
        {
            __android_log_write(ANDROID_LOG_ERROR, WEBRTC_LOG_TAG,
                                "Release encrypt sub-API failed");
            releaseOK = false;
        }
        else
        {
            veData.encrypt = NULL;
        }
    }

    return releaseOK;
}
