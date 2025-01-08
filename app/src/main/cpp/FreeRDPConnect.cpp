#include "FreeRDPConnect.h"

#include "log.h"

#include <freerdp/utils/signal.h>
#include <freerdp/client/cmdline.h>

struct VRContext
{
    rdpClientContext common;
};

namespace com::mefazm::rdvr {
    FreeRDPConnect::FreeRDPConnect(const std::string &hostname, const std::string &username,
                                   const std::string &password, const size_t w, const size_t h) {
        TAG = "com::mefazm::rdvr::FreeRDPConnect";
        RDP_CLIENT_ENTRY_POINTS clientEntryPoints = {
                .Size = sizeof(RDP_CLIENT_ENTRY_POINTS),
                .Version = RDP_CLIENT_INTERFACE_VERSION,
                .settings = nullptr,

                .GlobalInit = []() -> BOOL {
                    LOGI("GlobalInit()");
                    BOOL ret = FALSE;
                    if (freerdp_handle_signals() == 0) ret = TRUE;
                    LOGI("ret = %s", ret ? "TRUE" : "FALSE");
                    return ret;
                },

                .GlobalUninit = [](){
                    LOGI("GlobalUninit()");
                },

                .ContextSize = sizeof(VRContext),

                .ClientNew = [](freerdp *instance, rdpContext*) -> BOOL {
                    LOGI("ClientNew()");
                    if(nullptr == instance) throw std::invalid_argument(STRINGIFY(instance) " is nullptr.");

                    instance->PreConnect = [](freerdp* instance) -> BOOL {
                        LOGI("PreConnect()");
                        if(nullptr == instance) throw std::invalid_argument(STRINGIFY(instance) " is nullptr.");
                        if(nullptr == instance->context) throw std::invalid_argument(STRINGIFY(instance->context) " is nullptr.");
                        if(nullptr == instance->context->pubSub) throw std::invalid_argument(STRINGIFY(instance->context->pubSub) " is nullptr.");

                        if(CHANNEL_RC_OK != PubSub_SubscribeChannelConnected(instance->context->pubSub, [](void*, const ChannelConnectedEventArgs*){
                            LOGI("ChannelConnected()");
                        })) throw std::runtime_error("PubSub_SubscribeChannelConnected() fail.");

                        if(CHANNEL_RC_OK != PubSub_SubscribeChannelDisconnected(instance->context->pubSub, [](void*, const ChannelDisconnectedEventArgs*){
                            LOGI("ChannelDisconnected()");
                        })) throw std::runtime_error("PubSub_SubscribeChannelDisconnected() fail.");

                        return TRUE;
                    };

                    instance->PostConnect = [](freerdp* instance) -> BOOL {
                        LOGI("PostConnect()");
                        if(nullptr == instance) throw std::invalid_argument(STRINGIFY(instance) " is nullptr.");
                        if(nullptr == instance->context) throw std::invalid_argument(STRINGIFY(instance->context) " is nullptr.");
                        if(nullptr == instance->context->update) throw std::invalid_argument(STRINGIFY(instance->context->update) " is nullptr.");
                        if(nullptr == instance->context->graphics) throw std::invalid_argument(STRINGIFY(instance->context->graphics) " is nullptr.");

                        if(!gdi_init(instance, PIXEL_FORMAT_RGBX32)) throw std::runtime_error("gdi_init() fail.");

                        rdpPointer pointer = {};
                        pointer.size = sizeof(pointer);

                        pointer.New = [](rdpContext*, rdpPointer*) -> BOOL {
                            LOGI("PointerNew()");
                            return TRUE;
                        };

                        pointer.Free = [](rdpContext*, rdpPointer*) {
                            LOGI("PointerFree()");
                        };

                        pointer.Set = [](rdpContext*, rdpPointer*) -> BOOL {
                            LOGI("PointerSet()");
                            return TRUE;
                        };

                        pointer.SetNull = [](rdpContext*) -> BOOL {
                            LOGI("PointerSetNull()");
                            return TRUE;
                        };

                        pointer.SetDefault = [](rdpContext*) -> BOOL {
                            LOGI("PointerSetDefault()");
                            return TRUE;
                        };

                        pointer.SetPosition = [](rdpContext*, const UINT32 x, const UINT32 y) -> BOOL {
                            LOGI("PointerSetPosition(%u, %u)", x, y);
                            return TRUE;
                        };

                        graphics_register_pointer(instance->context->graphics, &pointer);

                        instance->context->update->BeginPaint = [](rdpContext*) -> BOOL {
                            LOGI("BeginPaint()");
                            return TRUE;
                        };

                        instance->context->update->EndPaint = [](rdpContext*) -> BOOL {
                            LOGI("EndPaint()");
                            return TRUE;
                        };

                        return TRUE;
                    };

                    instance->PostDisconnect = [](freerdp* instance){
                        LOGI("PostDisconnect()");
                        if(nullptr == instance) throw std::invalid_argument(STRINGIFY(instance) " is nullptr.");
                        gdi_free(instance);
                    };

                    instance->VerifyCertificateEx = [](freerdp*, const char*, UINT16, const char*, const char*, const char*, const char*, DWORD) -> DWORD {
                        LOGI("VerifyCertificateEx()");
                        return 1;
                    };

                    instance->VerifyChangedCertificateEx = [](freerdp*, const char*, UINT16, const char*, const char*, const char*, const char*, const char*, const char*, const char*, DWORD) -> DWORD {
                        LOGI("VerifyChangedCertificateEx()");
                        return 1;
                    };

                    return TRUE;
                },

                .ClientFree = [](freerdp*, rdpContext*){
                    LOGI("ClientFree()");
                },

                .ClientStart = [](rdpContext *context) -> int {
                    LOGI("ClientStart()");
                    auto vrc = reinterpret_cast<VRContext*>(context);
                    vrc->common.thread = CreateThread(nullptr, 0, [](LPVOID param) -> DWORD {
                        LOGI("FreeRDP main thread.");
                        if(nullptr == param) throw std::invalid_argument(STRINGIFY(param) " is nullptr.");
                        auto instance = reinterpret_cast<freerdp *>(param);
                        if(nullptr == instance->context) throw std::runtime_error(STRINGIFY(instance->context) " is nullptr.");
                        DWORD ret = 0;
                        if(freerdp_connect(instance)) {
                            LOGI("freerdp_connect() success.");
                            while(!freerdp_shall_disconnect_context(instance->context)) {
                                HANDLE handles[MAXIMUM_WAIT_OBJECTS] = {};
                                DWORD nCount = 0;
                                const DWORD n = freerdp_get_event_handles(instance->context, handles, MAXIMUM_WAIT_OBJECTS - nCount);
                                if(0 == n) throw std::runtime_error("freerdp_get_event_handles() fail.");
                                nCount += n;
                                if(WAIT_FAILED == WaitForMultipleObjects(nCount, handles, FALSE, INFINITE)) throw std::runtime_error("WaitForMultipleObjects() fail.");
                                if(!freerdp_check_event_handles(instance->context)) throw std::runtime_error("freerdp_check_event_handles() fail.");
                            }
                        } else {
                            ret = freerdp_get_last_error(instance->context);
                            LOGI("freerdp_connect() fail with error [%#010x].", ret);
                        }
                        return ret;
                    }, context->instance, 0, nullptr);
                    if(nullptr == vrc->common.thread) throw std::runtime_error(STRINGIFY(vrc->common.thread) " is nullptr.");
                    return 0;
                },

                .ClientStop = [](rdpContext* context) -> int {
                    LOGI("ClientStop()");
                    return freerdp_client_common_stop(context);
                }
        };
        context = freerdp_client_context_new(&clientEntryPoints);
        if(nullptr == context) throw std::runtime_error(STRINGIFY(context) " is nullptr.");
        LOGI("context = %p", context);
        if(nullptr == context->settings) throw std::runtime_error(STRINGIFY(context->settings) " is nullptr.");
        LOGI("context->settings = %p", context->settings);
        if(!freerdp_settings_set_string(context->settings, FreeRDP_ProxyHostname, nullptr)) throw std::runtime_error("freerdp_settings_set_string(FreeRDP_ProxyHostname) return FALSE.");
        if(!freerdp_settings_set_string(context->settings, FreeRDP_ProxyUsername, nullptr)) throw std::runtime_error("freerdp_settings_set_string(FreeRDP_ProxyUsername) return FALSE.");
        if(!freerdp_settings_set_string(context->settings, FreeRDP_ProxyPassword, nullptr)) throw std::runtime_error("freerdp_settings_set_string(FreeRDP_ProxyPassword) return FALSE.");
        if(!freerdp_set_connection_type(context->settings, CONNECTION_TYPE_AUTODETECT)) throw std::runtime_error("freerdp_set_connection_type(CONNECTION_TYPE_AUTODETECT) return FALSE.");
        if(!freerdp_settings_set_string(context->settings, FreeRDP_ServerHostname, hostname.c_str())) throw std::runtime_error("freerdp_settings_set_string(FreeRDP_ServerHostname) return FALSE.");
        if(!freerdp_settings_set_string(context->settings, FreeRDP_Username, username.c_str())) throw std::runtime_error("freerdp_settings_set_string(FreeRDP_Username) return FALSE.");
        if(!freerdp_settings_set_string(context->settings, FreeRDP_Domain, nullptr)) throw std::runtime_error("freerdp_settings_set_string(FreeRDP_Domain) return FALSE.");
        if(!freerdp_settings_set_string(context->settings, FreeRDP_Password, password.c_str())) throw std::runtime_error("freerdp_settings_set_string(FreeRDP_Password) return FALSE.");
        if(!freerdp_settings_set_uint32(context->settings, FreeRDP_DesktopWidth, w)) throw std::runtime_error("freerdp_settings_set_uint32(FreeRDP_DesktopWidth) return FALSE.");
        if(!freerdp_settings_set_uint32(context->settings, FreeRDP_DesktopHeight, h)) throw std::runtime_error("freerdp_settings_set_uint32(FreeRDP_DesktopHeight) return FALSE.");
        freerdp_settings_set_bool(context->settings, FreeRDP_CertificateCallbackPreferPEM, TRUE);
        freerdp_settings_set_uint32(context->settings, FreeRDP_OsMajorType, OSMAJORTYPE_ANDROID);
        freerdp_performance_flags_make(context->settings);
        if(!stream_dump_register_handlers(context, CONNECTION_STATE_MCS_CREATE_REQUEST, false)) throw std::runtime_error("stream_dump_register_handlers(CONNECTION_STATE_MCS_CREATE_REQUEST) return FALSE.");
        if(0 != freerdp_client_start(context)) throw std::runtime_error("freerdp_client_start() return not 0.");
    }

    void FreeRDPConnect::SetSize(const size_t w, const size_t h) {
        LOGI("SetSize(%zu, %zu)", w, h);
        if (nullptr != context && nullptr != context->settings) {
            if(!freerdp_settings_set_uint32(context->settings, FreeRDP_DesktopWidth, w)) throw std::runtime_error("freerdp_settings_set_uint32(FreeRDP_DesktopWidth) return FALSE.");
            if(!freerdp_settings_set_uint32(context->settings, FreeRDP_DesktopHeight, h)) throw std::runtime_error("freerdp_settings_set_uint32(FreeRDP_DesktopHeight) return FALSE.");
        }
        if(0 != freerdp_client_start(context)) throw std::runtime_error("freerdp_client_start() return not 0.");
    }
} // com::mefazm::rdvr
