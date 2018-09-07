#include <jni.h>
#include <stdlib.h>
#include "com_suicam_rtmp_RTMPJni.h"
#include "rtmp-client.h"

static void* s_rtmp;

JNIEXPORT jstring JNICALL Java_com_suicam_rtmp_RTMPJni_getVersion(JNIEnv *env, jclass cls)
{
	return (*env)->NewStringUTF(env, "Hello from native code!");
}

JNIEXPORT jint JNICALL Java_com_suicam_rtmp_RTMPJni_open(JNIEnv *env, jobject jthis, jstring jurl)
{
	const char* url;

	url = (*env)->GetStringUTFChars(env, jurl, NULL);
	if (NULL == url)
		return -1; // invalid parameter

	if (s_rtmp)
		rtmp_client_destroy(s_rtmp);

	s_rtmp = rtmp_client_create(url);
	(*env)->ReleaseStringUTFChars(env, jurl, url);

	return s_rtmp ? 0 : -1;
}

JNIEXPORT jint JNICALL Java_com_suicam_rtmp_RTMPJni_close(JNIEnv *env, jobject jthis)
{
	if (s_rtmp)
		rtmp_client_destroy(s_rtmp);
    s_rtmp = NULL;
	return 0;
}

JNIEXPORT jint JNICALL Java_com_suicam_rtmp_RTMPJni_sendVideo(JNIEnv *env, jobject jthis, jobject jByteBuffer, jlong pts, jlong dts)
{
	jclass cls = (*env)->GetObjectClass(env, jByteBuffer);
	jfieldID jlimit = (*env)->GetFieldID(env, cls, "limit", "I");
	jfieldID jposition = (*env)->GetFieldID(env, cls, "position", "I");
	if (NULL == jlimit || NULL == jposition)
		return -2;

	char *buf = (*env)->GetDirectBufferAddress(env, jByteBuffer);
	jlong capacity = (*env)->GetDirectBufferCapacity(env, jByteBuffer);
	jint position = (*env)->GetIntField(env, jByteBuffer, jposition);
	jint limit = (*env)->GetIntField(env, jByteBuffer, jlimit);
	if (NULL == buf || limit - position < 1 || limit > capacity || position < 0)
		return -3; // invalid parameter

	int r = rtmp_client_send_video(s_rtmp, buf + position, limit - position, (unsigned int)pts, (unsigned int)dts);

	// don't need delete local reference
	//(*env)->DeleteLocalRef(env, jlimit);
	//(*env)->DeleteLocalRef(env, jposition);
	return r;
}

JNIEXPORT jint JNICALL Java_com_suicam_rtmp_RTMPJni_sendAudio(JNIEnv *env, jobject jthis, jobject jByteBuffer, jlong pts, jlong dts)
{
	jclass cls = (*env)->GetObjectClass(env, jByteBuffer);
	jfieldID jlimit = (*env)->GetFieldID(env, cls, "limit", "I");
	jfieldID jposition = (*env)->GetFieldID(env, cls, "position", "I");
	if (NULL == jlimit || NULL == jposition)
		return -1;

	char *buf = (*env)->GetDirectBufferAddress(env, jByteBuffer);
	jlong capacity = (*env)->GetDirectBufferCapacity(env, jByteBuffer);
	jint position = (*env)->GetIntField(env, jByteBuffer, jposition);
	jint limit = (*env)->GetIntField(env, jByteBuffer, jlimit);
	if (NULL == buf || limit - position < 1 || limit > capacity || position < 0)
		return -1; // invalid parameter

	int r = rtmp_client_send_audio(s_rtmp, buf + position, limit - position, (unsigned int)pts, (unsigned int)dts);

	// don't need delete local reference
	//(*env)->DeleteLocalRef(env, jlimit);
	//(*env)->DeleteLocalRef(env, jposition);
	return r;
}

JNIEXPORT jint JNICALL Java_com_suicam_rtmp_RTMPJni_setHeader(JNIEnv *env, jobject jthis, jobject jByteBuffer, jint aacProfile, jint aacFrequence, jint aacChannel)
{
    jclass cls = (*env)->GetObjectClass(env, jByteBuffer);
    jfieldID jlimit = (*env)->GetFieldID(env, cls, "limit", "I");
    jfieldID jposition = (*env)->GetFieldID(env, cls, "position", "I");
    if (NULL == jlimit || NULL == jposition)
        return -1;

    char *buf = (*env)->GetDirectBufferAddress(env, jByteBuffer);
    jlong capacity = (*env)->GetDirectBufferCapacity(env, jByteBuffer);
    jint position = (*env)->GetIntField(env, jByteBuffer, jposition);
    jint limit = (*env)->GetIntField(env, jByteBuffer, jlimit);
    if (NULL == buf || limit - position < 1 || limit > capacity || position < 0)
        return -1; // invalid parameter

    static char s_buffer[64*1024];
    int r = rtmp_client_make_AVCDecoderConfigurationRecord(buf + position, limit - position, s_buffer, sizeof(s_buffer));
    if(r >= sizeof(s_buffer))
        return -1;

    switch (aacFrequence) {
        case 96000: aacFrequence = 0; break;
        case 88200: aacFrequence = 1; break;
        case 64000: aacFrequence = 2; break;
        case 48000: aacFrequence = 3; break;
        case 44100: aacFrequence = 4; break;
        case 32000: aacFrequence = 5; break;
        case 24000: aacFrequence = 6; break;
        case 16000: aacFrequence = 7; break;
        case 12000: aacFrequence = 8; break;
        case 11025: aacFrequence = 9; break;
        default: aacFrequence = 4; break;
    }

    // AudioSpecificConfig
    char p[2];
    p[0] = (aacProfile << 3) | ((aacFrequence >> 1) & 0x07);
    p[1] = ((aacProfile & 0x01) << 7) | (aacChannel << 3) | 0x00;

    r = rtmp_client_set_header(s_rtmp, p, 2, s_buffer, r);

    // don't need delete local reference
    //(*env)->DeleteLocalRef(env, jlimit);
    //(*env)->DeleteLocalRef(env, jposition);
    return r;
}