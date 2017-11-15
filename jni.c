#ifdef ENABLE_JNI

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <jni.h>
#include "aes.h"

#define DEF_JAVA(F) Java_jane_test_AesNative_ ## F

typedef struct // 16+244+4+8 = 16*17 = 272; must be aligned with 16 bytes
{
	jbyte buf[16];
	aes_encrypt_ctx ctx; // 4 * 60 + 4
	int left;
	void* base;
} jni_aes_ctx;

static void getByteArray(JNIEnv* jenv, jbyteArray buf, int pos, int len, jbyte* dst) // len must be > 0; dst must have len bytes available
{
	if(buf)
	{
		int bufLen, bufLeft;
		if(pos < 0) pos = 0;
		bufLen = (int)(*jenv)->GetArrayLength(jenv, buf);
		bufLeft = bufLen - pos;
		if(bufLeft >= len)
		{
			(*jenv)->GetByteArrayRegion(jenv, buf, pos, len, dst);
			return;
		}
		else if(bufLeft > 0)
		{
			(*jenv)->GetByteArrayRegion(jenv, buf, pos, bufLeft, dst);
			memset(dst + bufLeft, 0, len - bufLeft);
			return;
		}
	}
	memset(dst, 0, len);
}

// public static native long aes_create(byte[] keyBuf, int keyPos, byte[] ivBuf, int ivPos);
JNIEXPORT jlong JNICALL DEF_JAVA(aes_1create)(JNIEnv* jenv, jclass jcls, jbyteArray keyBuf, int keyPos, jbyteArray ivBuf, int ivPos)
{
	static volatile char s_init = 0;
	jni_aes_ctx* aes;
	jbyte key[16];
	void* base = malloc(sizeof(jni_aes_ctx) + 15);
	if(!base) return 0;

	aes = (jni_aes_ctx*)(((uintptr_t)base + 15) & ~(uintptr_t)15);
	aes->left = 0;
	aes->base = base;

	if(!s_init)
	{
		aes_init();
		s_init = 1;
	}

	getByteArray(jenv, keyBuf, keyPos, 16, key);
	getByteArray(jenv, ivBuf, ivPos, 16, aes->buf);
	aes_encrypt_key128(key, &aes->ctx);
	return (jlong)(uintptr_t)aes;
}

// public static native void aes_update(long handle_aes, byte[] buf, int pos, int len);
JNIEXPORT void JNICALL DEF_JAVA(aes_1update)(JNIEnv* jenv, jclass jcls, jlong handle_aes, jbyteArray buf, int pos, int len)
{
	jni_aes_ctx* aes;
	jbyte* p, *oldP, *pbuf;
	int bufLen, oldLen, left, n;
	jbyte b[3888];

	if(!handle_aes || !buf || len <= 0) return;
	if(pos < 0) pos = 0;
	aes = (jni_aes_ctx*)(uintptr_t)handle_aes;

	bufLen = (int)(*jenv)->GetArrayLength(jenv, buf);
	if((unsigned)(pos + len) > (unsigned)bufLen)
	{
		len = bufLen - pos;
		if(len <= 0) return;
	}
	oldP = p = (len <= sizeof(b) ? b : (jbyte*)malloc(len));
	if(!p) return;
	getByteArray(jenv, buf, pos, oldLen = len, p);

	left = aes->left;
	if(left > 0)
	{
		pbuf = aes->buf + 16 - left;
		n = (left < len ? left : len);
		left -= n;
		len -= n;
		if(n >= 8) { *(int64_t*)p ^= *(int64_t*)pbuf; p += 8; pbuf += 8; n -= 8; }
		if(n >= 4) { *(int    *)p ^= *(int    *)pbuf; p += 4; pbuf += 4; n -= 4; }
		if(n >= 2) { *(short  *)p ^= *(short  *)pbuf; p += 2; pbuf += 2; n -= 2; }
		if(n == 1) { *(char   *)p ^= *(char   *)pbuf; p++; }
		if(!len)
		{
			aes->left = left;
			goto end_;
		}
	}
loop_:
	aes_encrypt(aes->buf, aes->buf, &aes->ctx);
	if(len >= 16)
	{
		*(int64_t*)p ^= *(int64_t*) aes->buf;      p += 8;
		*(int64_t*)p ^= *(int64_t*)(aes->buf + 8); p += 8;
		if(!(len -= 16))
		{
			aes->left = 0;
			goto end_;
		}
		goto loop_;
	}
	pbuf = aes->buf;
	aes->left = 16 - len;
	if(len >= 8) { *(int64_t*)p ^= *(int64_t*)pbuf; p += 8; pbuf += 8; len -= 8; }
	if(len >= 4) { *(int    *)p ^= *(int    *)pbuf; p += 4; pbuf += 4; len -= 4; }
	if(len >= 2) { *(short  *)p ^= *(short  *)pbuf; p += 2; pbuf += 2; len -= 2; }
	if(len == 1)   *(char   *)p ^= *(char   *)pbuf;
end_:
	(*jenv)->SetByteArrayRegion(jenv, buf, pos, oldLen, oldP);
	if(oldP != b)
		free(oldP);
}

// public static native void aes_destroy(long handle_aes);
JNIEXPORT void JNICALL DEF_JAVA(aes_1destroy)(JNIEnv* jenv, jclass jcls, jlong handle_aes)
{
	if(handle_aes)
		free(((jni_aes_ctx*)(uintptr_t)handle_aes)->base);
}

#endif
