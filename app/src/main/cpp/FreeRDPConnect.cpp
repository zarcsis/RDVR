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
                                   const std::string &password) {
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
                .ClientNew = [](freerdp*, rdpContext*) -> BOOL {
                    return TRUE;
                    LOGI("ClientNew()");
                },
                .ClientFree = [](freerdp*, rdpContext*){
                    LOGI("ClientFree()");
                },
                .ClientStart = [](rdpContext*) -> int {
                    LOGI("ClientStart()");
                    return 0;
                },
                .ClientStop = [](rdpContext* context) -> int {
                    LOGI("ClientStop()");
                    return freerdp_client_common_stop(context);
                }
        };
        context = freerdp_client_context_new(&clientEntryPoints);
        if(nullptr == context) throw std::runtime_error(STRINGIFY(context) " is null.");
        LOGI("context = %p", context);
        if(nullptr == context->settings) throw std::runtime_error(STRINGIFY(context->settings) " is null.");
        LOGI("context->settings = %p", context->settings);
        freerdp_settings_set_string(context->settings, FreeRDP_ProxyHostname, nullptr);
        freerdp_settings_set_string(context->settings, FreeRDP_ProxyUsername, nullptr);
        freerdp_settings_set_string(context->settings, FreeRDP_ProxyPassword, nullptr);
        freerdp_set_connection_type(context->settings, CONNECTION_TYPE_AUTODETECT);
        freerdp_settings_set_string(context->settings, FreeRDP_ServerHostname, hostname.c_str());
        freerdp_settings_set_string(context->settings, FreeRDP_Username, username.c_str());
        freerdp_settings_set_string(context->settings, FreeRDP_Domain, nullptr);
        freerdp_settings_set_string(context->settings, FreeRDP_Password, password.c_str());
        freerdp_performance_flags_make(context->settings);
        stream_dump_register_handlers(context, CONNECTION_STATE_MCS_CREATE_REQUEST, false);
    }

    void FreeRDPConnect::SetSize(const size_t w, const size_t h) {
        LOGI("SetSize(%zu, %zu)", w, h);
        if (nullptr != context && nullptr != context->settings) {
            freerdp_settings_set_uint32(context->settings, FreeRDP_DesktopWidth, w);
            freerdp_settings_set_uint32(context->settings, FreeRDP_DesktopHeight, h);
        }
    }
} // com::mefazm::rdvr
