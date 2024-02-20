#pragma once

#include <Client.h>

static const int HTTP_SUCCESS =0;
static const int HTTP_ERROR_CONNECTION_FAILED =-1;

class HttpClient {
public:
    HttpClient(Client& aClient, const String& aServerName, uint16_t aServerPort) {}

    int post(const String& aURLPath,
            const String& aContentType,
            const String& aBody) {
        return HTTP_ERROR_CONNECTION_FAILED;
    }

    void connectionKeepAlive() { }
    int responseStatusCode() { return 404; }
    String responseBody() { return ""; }


};
