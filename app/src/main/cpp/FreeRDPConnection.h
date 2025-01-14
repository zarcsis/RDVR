#pragma once

#include <freerdp/freerdp.h>

#include <android/bitmap.h>

#include "string"

namespace com::mefazm::rdvr {
    class FreeRDPConnection {
    public:
        FreeRDPConnection(const std::string &hostname, const std::string &username,
                          const std::string &password);

        ~FreeRDPConnection();

        void setSize(const size_t w, const size_t h);

        void setBitmap(const jobject bmp);

        bool connect();

        bool disconnect();

    private:
        rdpContext *context = nullptr;
    };
} // com::mefazm::rdvr
