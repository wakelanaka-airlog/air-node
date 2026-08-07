#include "Esp32I2cBus.h"

#include "I2cBus.h"
#include <cstdio>
#include <esp_err.h>

namespace
{
    constexpr int kI2cTimeoutMs = 1000;
}

Esp32I2cBus::Esp32I2cBus(i2c_port_num_t port, gpio_num_t sdaPin, gpio_num_t sclPin, Logger& logger) : logger_{logger}
{
    i2c_master_bus_config_t busConfig{};
    busConfig.i2c_port = port;
    busConfig.sda_io_num = sdaPin;
    busConfig.scl_io_num = sclPin;
    busConfig.clk_source = I2C_CLK_SRC_DEFAULT;
    busConfig.glitch_ignore_cnt = 7;
    busConfig.flags.enable_internal_pullup = true;

    const esp_err_t result = i2c_new_master_bus(&busConfig, &busHandle_);
    if (result != ESP_OK)
    {
        char message[96];
        std::snprintf(message, sizeof(message), "Failed to initialize I2C bus: %s", esp_err_to_name(result));
        logger_.error(message);
        busHandle_ = nullptr;
    }
}

Esp32I2cBus::~Esp32I2cBus()
{
    for (auto& [_, deviceHandle] : deviceHandles_)
    {
        i2c_master_bus_rm_device(deviceHandle);
    }

    if (busHandle_ != nullptr)
    {
        i2c_del_master_bus(busHandle_);
    }
}

i2c_master_dev_handle_t Esp32I2cBus::getOrAddDevice(uint8_t address)
{
    auto it = deviceHandles_.find(address);
    if (it != deviceHandles_.end())
    {
        return it->second;
    }

    if (busHandle_ == nullptr)
    {
        return nullptr;
    }

    i2c_device_config_t deviceConfig{};
    deviceConfig.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    deviceConfig.device_address = address;
    deviceConfig.scl_speed_hz = 100000;

    i2c_master_dev_handle_t deviceHandle{};
    const esp_err_t result = i2c_master_bus_add_device(busHandle_, &deviceConfig, &deviceHandle);
    if (result != ESP_OK)
    {
        char message[96];
        std::snprintf(message, sizeof(message), "Failed to add I2C device 0x%02x: %s", address,
                      esp_err_to_name(result));
        logger_.error(message);
        return nullptr;
    }

    deviceHandles_.insert({address, deviceHandle});
    return deviceHandle;
}

bool Esp32I2cBus::write(uint8_t address, const uint8_t* data, size_t length)
{
    i2c_master_dev_handle_t device = getOrAddDevice(address);
    return device != nullptr && i2c_master_transmit(device, data, length, kI2cTimeoutMs) == ESP_OK;
}

bool Esp32I2cBus::read(uint8_t address, uint8_t* buffer, size_t length)
{
    i2c_master_dev_handle_t device = getOrAddDevice(address);
    return device != nullptr && i2c_master_receive(device, buffer, length, kI2cTimeoutMs) == ESP_OK;
}
