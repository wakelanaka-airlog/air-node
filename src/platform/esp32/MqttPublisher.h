#pragma once

#include <atomic>
#include <string>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "mqtt_client.h"

#include "Publisher.h"

class MqttPublisher : public Publisher
{
public:
    MqttPublisher(const char* brokerUri, const char* username, const char* password, const std::string& room);
    ~MqttPublisher() override;

    MqttPublisher(const MqttPublisher&) = delete;
    MqttPublisher& operator=(const MqttPublisher&) = delete;

    bool publish(const Measurement& measurement) override;

private:
    static void handleMqttEvent(void* handlerArgs, esp_event_base_t base, int32_t eventId, void* eventData);
    void publishOnlineStatus() const;

    esp_mqtt_client_handle_t client_;
    std::string topic_;
    std::string statusTopic_;
    std::atomic<bool> connected_{false};
    SemaphoreHandle_t publishAcked_;
    std::atomic<int> pendingMsgId_{-1};
};
