#include "FreeRDPConnection.h"
#include "log.h"
#include "tools.h"

#include <android/bitmap.h>

#include <stdexcept>
#include <memory>
#include <fstream>

static std::unique_ptr<com::mefazm::rdvr::FreeRDPConnection> pFreeRDPConnection = nullptr;

extern "C"
JNIEXPORT void JNICALL
Java_com_mefazm_rdvr_SessionActivity_connect(JNIEnv* env, jobject obj, jstring hostname, jstring username, jstring password, jstring home, jobject bmp) {
    if (nullptr == env) throw std::invalid_argument(STRINGIFY(env) " is nullptr.");
    if (nullptr == hostname) throw std::invalid_argument(STRINGIFY(hostname) " is nullptr.");
    if (nullptr == username) throw std::invalid_argument(STRINGIFY(username) " is nullptr.");
    if (nullptr == password) throw std::invalid_argument(STRINGIFY(password) " is nullptr.");
    if (nullptr == home) throw std::invalid_argument(STRINGIFY(home) " is nullptr.");

    TAG = getClassName(env, obj);
    setenv("HOME", strdup(jStringToString(env, home).c_str()), 1);
    pFreeRDPConnection = std::make_unique<com::mefazm::rdvr::FreeRDPConnection>(jStringToString(env, hostname), jStringToString(env, username), jStringToString(env, password));
    AndroidBitmapInfo inf = {};
    if (ANDROID_BITMAP_RESULT_SUCCESS != AndroidBitmap_getInfo(env, bmp, &inf)) throw std::runtime_error("AndroidBitmap_getInfo() fail.");
    pFreeRDPConnection->setSize(inf.width, inf.height);
    if(!pFreeRDPConnection->connect()) LOGI("Connect fail.");
}

extern "C"
JNIEXPORT void JNICALL
Java_com_mefazm_rdvr_SessionActivity_resize(JNIEnv *, jobject, jint width, jint height) {
    if (1 > width) throw std::invalid_argument(STRINGIFY(width) " < 1.");
    if (1 > height) throw std::invalid_argument(STRINGIFY(height) " < 1.");
    LOGI("width: \"%d\", height: \"%d\"", width, height);
    if (nullptr != pFreeRDPConnection) pFreeRDPConnection->setSize(width, height);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_mefazm_rdvr_SessionActivity_disconnect(JNIEnv *, jobject) {
    if(nullptr != pFreeRDPConnection && !pFreeRDPConnection->disconnect()) LOGI("Disconnect fail.");
}
