#include <jni.h>
#include <stdint.h>
#include <stdlib.h>

extern void randuuid(uint8_t*, size_t);

/*
 * Class:     net_herle_random_mtarc4random
 * Method:    randuuid
 * Signature: ([B)V
 */
JNIEXPORT void JNICALL Java_net_herle_random_mtarc4random_randuuid
  (JNIEnv *env, jobject obj, jbyteArray arr)
{
    uint8_t buf[16];

    randuuid(buf, sizeof buf);

    (*env)->SetByteArrayRegion(env, arr, 0, 16, (const jbyte*) &buf[0]);
}

