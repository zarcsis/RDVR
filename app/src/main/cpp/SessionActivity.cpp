#include "FreeRDPConnect.h"
#include "log.h"
#include "tools.h"

#include <stdexcept>
#include <memory>

static std::unique_ptr<com::mefazm::rdvr::FreeRDPConnect> pFreeRDPConnect = nullptr;

extern "C"
JNIEXPORT void JNICALL
Java_com_mefazm_rdvr_SessionActivity_connect(JNIEnv* env, jobject obj, jstring hostname, jstring username, jstring password, jstring home) {
    if (nullptr == env) throw std::invalid_argument(STRINGIFY(env) " is null.");
    if (nullptr == hostname) throw std::invalid_argument(STRINGIFY(hostname) " is null.");
    if (nullptr == username) throw std::invalid_argument(STRINGIFY(username) " is null.");
    if (nullptr == password) throw std::invalid_argument(STRINGIFY(password) " is null.");
    if (nullptr == home) throw std::invalid_argument(STRINGIFY(home) " is null.");

    TAG = getClassName(env, obj);
    LOGI("hostname: \"%s\", username: \"%s\", password: \"%s\", home: \"%s\"",
         jStringToString(env, hostname).c_str(),
         jStringToString(env, username).c_str(),
         jStringToString(env, password).c_str(),
         jStringToString(env, home).c_str());
    setenv("HOME", strdup(jStringToString(env, home).c_str()), 1);
    pFreeRDPConnect = std::make_unique<com::mefazm::rdvr::FreeRDPConnect>(jStringToString(env, hostname), jStringToString(env, username), jStringToString(env, password));
}

extern "C"
JNIEXPORT void JNICALL
Java_com_mefazm_rdvr_SessionActivity_resize(JNIEnv *, jobject, jint width, jint height) {
    if (1 > width) throw std::invalid_argument(STRINGIFY(width) " < 1.");
    if (1 > height) throw std::invalid_argument(STRINGIFY(height) " < 1.");
    LOGI("width: \"%d\", height: \"%d\"", width, height);
    if (nullptr != pFreeRDPConnect) pFreeRDPConnect->SetSize(width, height);
}
