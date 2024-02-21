#pragma once

#include <Client.h>

#define MQTT_CONNECTION_REFUSED            -2
#define MQTT_CONNECTION_TIMEOUT            -1
#define MQTT_SUCCESS                        0
#define MQTT_UNACCEPTABLE_PROTOCOL_VERSION  1
#define MQTT_IDENTIFIER_REJECTED            2
#define MQTT_SERVER_UNAVAILABLE             3
#define MQTT_BAD_USER_NAME_OR_PASSWORD      4
#define MQTT_NOT_AUTHORIZED                 5

class MqttClient : public Client {
public:
    MqttClient(Client& client) {}

    uint8_t connected() {
        // Actually returns a bool
        // return clientConnected() && _connected;
        return false;
    }

    int connectError() const {
        return MQTT_CONNECTION_REFUSED;
    }

    void setUsernamePassword(const String& username, const String& password) {

    }

    int beginMessage(const String& topic, bool retain = false, uint8_t qos = 0, bool dup = false) {
        return MQTT_CONNECTION_REFUSED;
    }

    int endMessage() {}

    void stop() {}

    size_t print(const String &s) {
        return 0;
    }

    void poll() {}

};
