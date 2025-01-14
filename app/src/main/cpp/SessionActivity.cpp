#include "FreeRDPConnection.h"
#include "log.h"
#include "tools.h"

#include <android/bitmap.h>

#include <stdexcept>
#include <memory>
#include <fstream>

static std::unique_ptr<com::mefazm::rdvr::FreeRDPConnection> pFreeRDPConnection = nullptr;
JavaVM *gJVM = nullptr;

extern "C"
JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM *vm, void *) {
    TAG = "com.mefazm.rdvr.SessionActivity";
    LOGI("JNI_OnLoad()");
    gJVM = vm;
    return JNI_VERSION_1_6;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_mefazm_rdvr_SessionActivity_connect(JNIEnv *env, jobject, jstring hostname,
                                             jstring username, jstring password, jstring home,
                                             jobject bmp) {
    LOGI("Java_com_mefazm_rdvr_SessionActivity_connect()");
    if (nullptr == env) throw std::invalid_argument(STRINGIFY(env) " is nullptr.");
    if (nullptr == hostname) throw std::invalid_argument(STRINGIFY(hostname) " is nullptr.");
    if (nullptr == username) throw std::invalid_argument(STRINGIFY(username) " is nullptr.");
    if (nullptr == password) throw std::invalid_argument(STRINGIFY(password) " is nullptr.");
    if (nullptr == home) throw std::invalid_argument(STRINGIFY(home) " is nullptr.");

    setenv("HOME", strdup(jStringToString(env, home).c_str()), 1);
    pFreeRDPConnection = std::make_unique<com::mefazm::rdvr::FreeRDPConnection>(
            jStringToString(env, hostname), jStringToString(env, username),
            jStringToString(env, password));
    pFreeRDPConnection->setBitmap(bmp);
    if (!pFreeRDPConnection->connect()) LOGI("Connect fail.");
}

extern "C"
JNIEXPORT void JNICALL
Java_com_mefazm_rdvr_SessionActivity_resize(JNIEnv *, jobject, jint width, jint height) {
    LOGI("Java_com_mefazm_rdvr_SessionActivity_resize()");
    if (1 > width) throw std::invalid_argument(STRINGIFY(width) " < 1.");
    if (1 > height) throw std::invalid_argument(STRINGIFY(height) " < 1.");
    LOGI("width: \"%d\", height: \"%d\"", width, height);
    if (nullptr != pFreeRDPConnection) pFreeRDPConnection->setSize(width, height);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_mefazm_rdvr_SessionActivity_disconnect(JNIEnv *, jobject) {
    LOGI("Java_com_mefazm_rdvr_SessionActivity_disconnect()");
    if (nullptr != pFreeRDPConnection && !pFreeRDPConnection->disconnect())
        LOGI("Disconnect fail.");
    pFreeRDPConnection = nullptr;
}
