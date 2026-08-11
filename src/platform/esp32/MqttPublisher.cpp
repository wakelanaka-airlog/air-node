#include "MqttPublisher.h"

#include <cstring>

#include "mqtt/AirNodeTopic.h"
#include "mqtt/MeasurementPayload.h"

namespace
{
    constexpr uint32_t kPublishAckTimeoutMs = 5000;
    constexpr const char* kOnlinePayload = "online";
    constexpr const char* kOfflinePayload = "offline";
    constexpr int kStatusQos = 1;
    constexpr int kKeepaliveSeconds = 15;
}

MqttPublisher::MqttPublisher(const char* brokerUri, const char* username, const char* password,
                             const std::string& room)
    : topic_{AirNodeTopic::measurementTopic(room)}
    , statusTopic_{AirNodeTopic::statusTopic(room)}
{
    publishAcked_ = xSemaphoreCreateBinary();

    esp_mqtt_client_config_t mqttConfig = {};
    mqttConfig.broker.address.uri = brokerUri;
    mqttConfig.credentials.username = username;
    mqttConfig.credentials.authentication.password = password;
    mqttConfig.session.keepalive = kKeepaliveSeconds;
    mqttConfig.session.last_will.topic = statusTopic_.c_str();
    mqttConfig.session.last_will.msg = kOfflinePayload;
    mqttConfig.session.last_will.msg_len = 0; // 0 -> broker uses strlen(msg)
    mqttConfig.session.last_will.qos = kStatusQos;
    mqttConfig.session.last_will.retain = true;

    client_ = esp_mqtt_client_init(&mqttConfig);
    esp_mqtt_client_register_event(client_, MQTT_EVENT_ANY, &MqttPublisher::handleMqttEvent, this);
    esp_mqtt_client_start(client_);
}

MqttPublisher::~MqttPublisher()
{
    esp_mqtt_client_stop(client_);
    esp_mqtt_client_destroy(client_);
    vSemaphoreDelete(publishAcked_);
}

void MqttPublisher::handleMqttEvent(void* handlerArgs, esp_event_base_t, int32_t eventId, void* eventData)
{
    auto* self = static_cast<MqttPublisher*>(handlerArgs);
    switch (static_cast<esp_mqtt_event_id_t>(eventId))
    {
    case MQTT_EVENT_CONNECTED:
        self->connected_ = true;
        self->publishOnlineStatus();
        break;
    case MQTT_EVENT_DISCONNECTED:
        self->connected_ = false;
        break;
    case MQTT_EVENT_PUBLISHED:
        {
            auto* event = static_cast<esp_mqtt_event_handle_t>(eventData);
            if (event->msg_id == self->pendingMsgId_)
            {
                xSemaphoreGive(self->publishAcked_);
            }
            break;
        }
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
    std::size_t length = 0;
    if (!formatMeasurementPayload(measurement, payload, sizeof(payload), length))
    {
        return false;
    }

    xSemaphoreTake(publishAcked_, 0);

    pendingMsgId_ = esp_mqtt_client_publish(client_, topic_.c_str(), payload, static_cast<int>(length), 1, 0);
    if (pendingMsgId_ == -1)
    {
        return false;
    }

    const bool acked = xSemaphoreTake(publishAcked_, pdMS_TO_TICKS(kPublishAckTimeoutMs)) == pdTRUE;
    if (!acked)
    {
        pendingMsgId_ = -1;
    }
    return acked;
}

void MqttPublisher::publishOnlineStatus()
{
    esp_mqtt_client_publish(client_, statusTopic_.c_str(), kOnlinePayload,
                             static_cast<int>(std::strlen(kOnlinePayload)), kStatusQos, true);
}
