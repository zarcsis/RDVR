#pragma once

#include <freerdp/freerdp.h>

#include "string"

namespace com::mefazm::rdvr {
    class FreeRDPConnect {
    public:
        FreeRDPConnect(const std::string& hostname, const std::string& username, const std::string& password);
        void SetSize(const size_t w, const size_t h);
    private:
        rdpContext* context = nullptr;
    };
} // com::mefazm::rdvr
