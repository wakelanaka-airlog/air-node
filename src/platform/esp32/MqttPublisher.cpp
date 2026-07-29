#include "MqttPublisher.h"

#include <cstdio>

MqttPublisher::MqttPublisher(const char* brokerUri, const char* username, const char* password,
                             std::string topic, std::string room)
    : topic_{std::move(topic)}, room_{std::move(room)}
{
    esp_mqtt_client_config_t mqttConfig = {};
    mqttConfig.broker.address.uri = brokerUri;
    mqttConfig.credentials.username = username;
    mqttConfig.credentials.authentication.password = password;

    client_ = esp_mqtt_client_init(&mqttConfig);
    esp_mqtt_client_register_event(client_, MQTT_EVENT_ANY, &MqttPublisher::handleMqttEvent, this);
    esp_mqtt_client_start(client_);
}

MqttPublisher::~MqttPublisher()
{
    esp_mqtt_client_stop(client_);
    esp_mqtt_client_destroy(client_);
}

void MqttPublisher::handleMqttEvent(void* handlerArgs, esp_event_base_t, int32_t eventId, void*)
{
    auto* self = static_cast<MqttPublisher*>(handlerArgs);
    switch (static_cast<esp_mqtt_event_id_t>(eventId))
    {
    case MQTT_EVENT_CONNECTED:
        self->connected_ = true;
        break;
    case MQTT_EVENT_DISCONNECTED:
        self->connected_ = false;
        break;
    default:
        break;
    }
}

bool MqttPublisher::publish(const Measurement& measurement)
{
    if (!connected_)
    {
        return false;
    }

    char payload[192];
    const int length = snprintf(payload, sizeof(payload),
                                "{\"room\":\"%s\", \"timestamp\":%llu,\"co2Ppm\":%u, \"temperatureCelsius\":%.2f,\"humidityPercent\":%.2f}",
                                room_.c_str(), static_cast<unsigned long long>(measurement.timestampUnixMillis),
                                measurement.co2Ppm, measurement.temperatureCelsius, measurement.humidityPercent);

    const int messageId = esp_mqtt_client_publish(client_, topic_.c_str(), payload, length, 1, 0);
    return messageId != -1;
}
