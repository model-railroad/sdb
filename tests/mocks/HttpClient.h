/*
 * Project: Software Defined Blocks
 * Copyright (C) 2024 alf.labs gmail com.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <Client.h>

static const int HTTP_SUCCESS =0;
static const int HTTP_ERROR_CONNECTION_FAILED =-1;

class HttpClient : public Client {
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
