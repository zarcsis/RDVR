#include "FreeRDPConnection.h"

#include "log.h"

#include <freerdp/utils/signal.h>
#include <freerdp/client/cmdline.h>

extern JavaVM *gJVM;

struct VRContext {
    rdpClientContext common{};
    std::atomic_bool connected = false;
    jobject bmp = nullptr;
};

namespace com::mefazm::rdvr {
    static JNIEnv *getEnv() {
        if (nullptr == gJVM) throw std::runtime_error("gJVM is nullptr.");
        void *vEnv = nullptr;
        if (JNI_OK != gJVM->GetEnv(&vEnv, JNI_VERSION_1_6)) {
            JNIEnv *env = nullptr;
            if (JNI_OK != gJVM->AttachCurrentThread(&env, nullptr))
                throw std::runtime_error("gJVM->AttachCurrentThread() fail.");
            if (JNI_OK != gJVM->GetEnv(&vEnv, JNI_VERSION_1_6))
                throw std::runtime_error("gJVM->GetEnv() fail.");
        }
        if (nullptr == vEnv) throw std::runtime_error("vEnv is nullptr.");
        return reinterpret_cast<JNIEnv *>(vEnv);
    }

    FreeRDPConnection::FreeRDPConnection(const std::string &hostname, const std::string &username,
                                         const std::string &password) {
        TAG = "com.mefazm.rdvr.FreeRDPConnection";
        LOGI("FreeRDPConnection()");
        RDP_CLIENT_ENTRY_POINTS clientEntryPoints = {
                .Size = sizeof(RDP_CLIENT_ENTRY_POINTS),
                .Version = RDP_CLIENT_INTERFACE_VERSION,
                .settings = nullptr,

                .GlobalInit = []() -> BOOL {
                    LOGI("GlobalInit()");
                    return freerdp_handle_signals() == 0;
                },

                .GlobalUninit = []() {
                    LOGI("GlobalUninit()");
                },

                .ContextSize = sizeof(VRContext),

                .ClientNew = [](freerdp *instance, rdpContext *) -> BOOL {
                    LOGI("ClientNew()");
                    if (nullptr == instance)
                        throw std::invalid_argument(STRINGIFY(instance) " is nullptr.");

                    instance->PreConnect = [](freerdp *instance) -> BOOL {
                        LOGI("PreConnect()");
                        if (nullptr == instance)
                            throw std::invalid_argument(STRINGIFY(instance) " is nullptr.");
                        if (nullptr == instance->context)
                            throw std::invalid_argument(
                                    STRINGIFY(instance->context) " is nullptr.");
                        if (nullptr == instance->context->pubSub)
                            throw std::invalid_argument(
                                    STRINGIFY(instance->context->pubSub) " is nullptr.");

                        if (CHANNEL_RC_OK !=
                            PubSub_SubscribeChannelConnected(instance->context->pubSub,
                                                             [](void *context,
                                                                const ChannelConnectedEventArgs *e) {
                                                                 LOGI("ChannelConnected()");
                                                                 freerdp_client_OnChannelConnectedEventHandler(
                                                                         context, e);
                                                             }))
                            throw std::runtime_error("PubSub_SubscribeChannelConnected() fail.");

                        if (CHANNEL_RC_OK !=
                            PubSub_SubscribeChannelDisconnected(instance->context->pubSub,
                                                                [](void *context,
                                                                   const ChannelDisconnectedEventArgs *e) {
                                                                    LOGI("ChannelDisconnected()");
                                                                    freerdp_client_OnChannelDisconnectedEventHandler(
                                                                            context, e);
                                                                }))
                            throw std::runtime_error("PubSub_SubscribeChannelDisconnected() fail.");

                        return TRUE;
                    };

                    instance->PostConnect = [](freerdp *instance) -> BOOL {
                        LOGI("PostConnect()");
                        if (nullptr == instance)
                            throw std::invalid_argument(STRINGIFY(instance) " is nullptr.");
                        if (nullptr == instance->context)
                            throw std::invalid_argument(
                                    STRINGIFY(instance->context) " is nullptr.");
                        if (nullptr == instance->context->update)
                            throw std::invalid_argument(
                                    STRINGIFY(instance->context->update) " is nullptr.");
                        if (nullptr == instance->context->graphics)
                            throw std::invalid_argument(
                                    STRINGIFY(instance->context->graphics) " is nullptr.");

                        if (!gdi_init(instance, PIXEL_FORMAT_RGBX32))
                            throw std::runtime_error("gdi_init() fail.");

                        rdpPointer pointer = {};
                        pointer.size = sizeof(pointer);

                        pointer.New = [](rdpContext *, rdpPointer *) -> BOOL {
                            LOGI("PointerNew()");
                            return TRUE;
                        };

                        pointer.Free = [](rdpContext *, rdpPointer *) {
                            LOGI("PointerFree()");
                        };

                        pointer.Set = [](rdpContext *, rdpPointer *) -> BOOL {
                            LOGI("PointerSet()");
                            return TRUE;
                        };

                        pointer.SetNull = [](rdpContext *) -> BOOL {
                            LOGI("PointerSetNull()");
                            return TRUE;
                        };

                        pointer.SetDefault = [](rdpContext *) -> BOOL {
                            LOGI("PointerSetDefault()");
                            return TRUE;
                        };

                        pointer.SetPosition = [](rdpContext *, const UINT32 x,
                                                 const UINT32 y) -> BOOL {
                            LOGI("PointerSetPosition(%u, %u)", x, y);
                            return TRUE;
                        };

                        graphics_register_pointer(instance->context->graphics, &pointer);

                        instance->context->update->BeginPaint = [](rdpContext *) -> BOOL {
                            LOGI("BeginPaint()");
                            return TRUE;
                        };

                        instance->context->update->EndPaint = [](rdpContext *context) -> BOOL {
                            LOGI("EndPaint()");
                            if (nullptr == context)
                                throw std::runtime_error(STRINGIFY(context) " is nullptr.");
                            if (nullptr == context->gdi)
                                throw std::runtime_error(STRINGIFY(context->gdi) " is nullptr.");
                            if (nullptr == context->gdi->primary)
                                throw std::runtime_error(
                                        STRINGIFY(context->gdi->primary) " is nullptr.");
                            if (nullptr == context->gdi->primary_buffer)
                                throw std::runtime_error(
                                        STRINGIFY(context->gdi->primary_buffer) " is nullptr.");
                            if (nullptr == context->gdi->primary->hdc)
                                throw std::runtime_error(
                                        STRINGIFY(context->gdi->primary->hdc) " is nullptr.");
                            if (nullptr == context->gdi->primary->hdc->hwnd)
                                throw std::runtime_error(
                                        STRINGIFY(context->gdi->primary->hdc->hwnd) " is nullptr.");
                            auto hwnd = context->gdi->primary->hdc->hwnd;
                            if (1 <= hwnd->ninvalid && nullptr == hwnd->cinvalid)
                                throw std::runtime_error(STRINGIFY(hwnd->cinvalid) " is nullptr.");
                            auto env = getEnv();
                            auto vrc = reinterpret_cast<VRContext *>(context);
                            void *data = nullptr;
                            AndroidBitmapInfo inf = {};
                            if (ANDROID_BITMAP_RESULT_SUCCESS !=
                                AndroidBitmap_getInfo(env, vrc->bmp, &inf))
                                throw std::runtime_error("AndroidBitmap_getInfo() fail.");
                            for (auto i = 0; i < hwnd->ninvalid; ++i) {
                                if (ANDROID_BITMAP_RESULT_SUCCESS !=
                                    AndroidBitmap_lockPixels(env, vrc->bmp, &data))
                                    throw std::runtime_error("AndroidBitmap_lockPixels() fail.");
                                if (nullptr == data)
                                    throw std::runtime_error(STRINGIFY(data) " is nullptr.");
                                LOGI("Paint: x = %d, y = %d, w = %d, h = %d", hwnd->cinvalid[i].x,
                                     hwnd->cinvalid[i].y, hwnd->cinvalid[i].w, hwnd->cinvalid[i].h);
                                if (!freerdp_image_copy(reinterpret_cast<BYTE *>(data),
                                                        PIXEL_FORMAT_RGBX32, inf.stride,
                                                        hwnd->cinvalid[i].x, hwnd->cinvalid[i].y,
                                                        hwnd->cinvalid[i].w, hwnd->cinvalid[i].h,
                                                        context->gdi->primary_buffer,
                                                        context->gdi->dstFormat,
                                                        context->gdi->stride, hwnd->cinvalid[i].x,
                                                        hwnd->cinvalid[i].y, &context->gdi->palette,
                                                        FREERDP_FLIP_NONE))
                                    throw std::runtime_error("freerdp_image_copy() fail.");
                                if (ANDROID_BITMAP_RESULT_SUCCESS !=
                                    AndroidBitmap_unlockPixels(env, vrc->bmp))
                                    throw std::runtime_error("AndroidBitmap_unlockPixels() fail.");
                            }
                            context->gdi->primary->hdc->hwnd->invalid->null = TRUE;
                            context->gdi->primary->hdc->hwnd->ninvalid = 0;
                            return TRUE;
                        };

                        instance->context->update->DesktopResize = [](rdpContext *) -> BOOL {
                            LOGI("DesktopResize()");
                            return TRUE;
                        };

                        return TRUE;
                    };

                    instance->PostDisconnect = [](freerdp *instance) {
                        LOGI("PostDisconnect()");
                        if (nullptr == instance)
                            throw std::invalid_argument(STRINGIFY(instance) " is nullptr.");
                        gdi_free(instance);
                    };

                    instance->VerifyCertificateEx = [](freerdp *, const char *, UINT16,
                                                       const char *, const char *, const char *,
                                                       const char *, DWORD) -> DWORD {
                        LOGI("VerifyCertificateEx()");
                        return 1;
                    };

                    instance->VerifyChangedCertificateEx = [](freerdp *, const char *, UINT16,
                                                              const char *, const char *,
                                                              const char *, const char *,
                                                              const char *, const char *,
                                                              const char *, DWORD) -> DWORD {
                        LOGI("VerifyChangedCertificateEx()");
                        return 1;
                    };

                    return TRUE;
                },

                .ClientFree = [](freerdp *, rdpContext *) {
                    LOGI("ClientFree()");
                },

                .ClientStart = [](rdpContext *context) -> int {
                    LOGI("ClientStart()");
                    auto cc = reinterpret_cast<rdpClientContext *>(context);
                    LOGI("cc = %p", cc);

                    cc->thread = CreateThread(nullptr, 0, [](LPVOID param) -> DWORD {
                        LOGI("FreeRDP main thread.");
                        if (nullptr == param)
                            throw std::invalid_argument(STRINGIFY(param) " is nullptr.");
                        auto instance = reinterpret_cast<freerdp *>(param);
                        if (nullptr == instance->context)
                            throw std::runtime_error(STRINGIFY(instance->context) " is nullptr.");
                        DWORD ret = 0;
                        if (freerdp_connect(instance)) {
                            LOGI("freerdp_connect() success.");
                            auto vrc = reinterpret_cast<VRContext *>(instance->context);
                            LOGI("vrc = %p", vrc);
                            vrc->connected = true;
                            while (!freerdp_shall_disconnect_context(instance->context)) {
                                HANDLE handles[MAXIMUM_WAIT_OBJECTS] = {};
                                DWORD nCount = 0;
                                const DWORD n = freerdp_get_event_handles(instance->context,
                                                                          handles,
                                                                          MAXIMUM_WAIT_OBJECTS -
                                                                          nCount);
                                if (0 == n)
                                    throw std::runtime_error("freerdp_get_event_handles() fail.");
                                nCount += n;
                                if (WAIT_FAILED ==
                                    WaitForMultipleObjects(nCount, handles, FALSE, INFINITE))
                                    throw std::runtime_error("WaitForMultipleObjects() fail.");
                                if (!freerdp_check_event_handles(instance->context)) {
                                    ret = freerdp_get_last_error(instance->context);
                                    break;
                                }
                            }
                            if (!freerdp_disconnect(instance))
                                throw std::runtime_error("freerdp_disconnect() fail");
                            vrc->connected = false;
                        } else {
                            ret = freerdp_get_last_error(instance->context);
                            LOGI("freerdp_connect() fail with error [%#010x].", ret);
                        }
                        return ret;
                    }, context->instance, 0, nullptr);

                    if (nullptr == cc->thread)
                        throw std::runtime_error(STRINGIFY(cc->thread) " is nullptr.");
                    return 0;
                },

                .ClientStop = [](rdpContext *context) -> int {
                    LOGI("ClientStop()");
                    LOGI("context = %p", context);
                    return freerdp_client_common_stop(context);
                }
        };
        context = freerdp_client_context_new(&clientEntryPoints);
        if (nullptr == context) throw std::runtime_error(STRINGIFY(context) " is nullptr.");
        if (nullptr == context->settings)
            throw std::runtime_error(STRINGIFY(context->settings) " is nullptr.");
        if (!freerdp_settings_set_string(context->settings, FreeRDP_ProxyHostname, nullptr))
            throw std::runtime_error(
                    "freerdp_settings_set_string(FreeRDP_ProxyHostname) return FALSE.");
        if (!freerdp_settings_set_string(context->settings, FreeRDP_ProxyUsername, nullptr))
            throw std::runtime_error(
                    "freerdp_settings_set_string(FreeRDP_ProxyUsername) return FALSE.");
        if (!freerdp_settings_set_string(context->settings, FreeRDP_ProxyPassword, nullptr))
            throw std::runtime_error(
                    "freerdp_settings_set_string(FreeRDP_ProxyPassword) return FALSE.");
        if (!freerdp_set_connection_type(context->settings, CONNECTION_TYPE_AUTODETECT))
            throw std::runtime_error(
                    "freerdp_set_connection_type(CONNECTION_TYPE_AUTODETECT) return FALSE.");
        if (!freerdp_settings_set_string(context->settings, FreeRDP_ServerHostname,
                                         hostname.c_str()))
            throw std::runtime_error(
                    "freerdp_settings_set_string(FreeRDP_ServerHostname) return FALSE.");
        if (!freerdp_settings_set_string(context->settings, FreeRDP_Username,
                                         username.c_str()))
            throw std::runtime_error("freerdp_settings_set_string(FreeRDP_Username) return FALSE.");
        if (!freerdp_settings_set_string(context->settings, FreeRDP_Domain, nullptr))
            throw std::runtime_error("freerdp_settings_set_string(FreeRDP_Domain) return FALSE.");
        if (!freerdp_settings_set_string(context->settings, FreeRDP_Password,
                                         password.c_str()))
            throw std::runtime_error("freerdp_settings_set_string(FreeRDP_Password) return FALSE.");
        freerdp_settings_set_bool(context->settings, FreeRDP_CertificateCallbackPreferPEM, TRUE);
        freerdp_settings_set_uint32(context->settings, FreeRDP_OsMajorType, OSMAJORTYPE_ANDROID);
        freerdp_performance_flags_make(context->settings);
        if (!stream_dump_register_handlers(context, CONNECTION_STATE_MCS_CREATE_REQUEST, false))
            throw std::runtime_error(
                    "stream_dump_register_handlers(CONNECTION_STATE_MCS_CREATE_REQUEST) return FALSE.");
    }

    FreeRDPConnection::~FreeRDPConnection() {
        LOGI("~FreeRDPConnection()");
        if (nullptr != context) {
            auto vrc = reinterpret_cast<VRContext *>(context);
            if (vrc->connected) disconnect();
            freerdp_client_context_free(context);
            context = nullptr;
            if (nullptr != vrc->bmp) {
                getEnv()->DeleteGlobalRef(vrc->bmp);
                vrc->bmp = nullptr;
            }
        }
    }

    void FreeRDPConnection::setSize(const size_t w, const size_t h) {
        LOGI("setSize(%zu, %zu)", w, h);
        if (nullptr != context && nullptr != context->settings) {
            if (!freerdp_settings_set_uint32(context->settings, FreeRDP_DesktopWidth, w))
                throw std::runtime_error(
                        "freerdp_settings_set_uint32(FreeRDP_DesktopWidth) return FALSE.");
            if (!freerdp_settings_set_uint32(context->settings, FreeRDP_DesktopHeight, h))
                throw std::runtime_error(
                        "freerdp_settings_set_uint32(FreeRDP_DesktopHeight) return FALSE.");
        }
    }

    bool FreeRDPConnection::connect() {
        LOGI("connect()");
        return 0 == freerdp_client_start(context);
    }

    bool FreeRDPConnection::disconnect() {
        LOGI("disconnect()");
        return 0 == freerdp_client_stop(context);
    }

    void FreeRDPConnection::setBitmap(const jobject bmp) {
        LOGI("setBitmap");
        auto env = getEnv();
        auto vrc = reinterpret_cast<VRContext *>(context);
        vrc->bmp = env->NewGlobalRef(bmp);
        if (nullptr == vrc->bmp) throw std::runtime_error(STRINGIFY(vrc->bmp) " is nullptr.");
        AndroidBitmapInfo inf = {};
        if (ANDROID_BITMAP_RESULT_SUCCESS != AndroidBitmap_getInfo(env, bmp, &inf))
            throw std::runtime_error("AndroidBitmap_getInfo() fail.");
        setSize(inf.width, inf.height);
    }
} // com::mefazm::rdvr
