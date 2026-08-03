#include "Esp32I2cBus.h"

#include "I2cBus.h"
#include <esp_err.h>

namespace
{
    constexpr int kI2cTimeoutMs = 1000;
}

Esp32I2cBus::Esp32I2cBus(i2c_port_num_t port, gpio_num_t sdaPin, gpio_num_t sclPin)
{
    i2c_master_bus_config_t busConfig{};
    busConfig.i2c_port = port;
    busConfig.sda_io_num = sdaPin;
    busConfig.scl_io_num = sclPin;
    busConfig.clk_source = I2C_CLK_SRC_DEFAULT;
    busConfig.glitch_ignore_cnt = 7;
    busConfig.flags.enable_internal_pullup = true;

    i2c_new_master_bus(&busConfig, &busHandle_);
}

Esp32I2cBus::~Esp32I2cBus()
{
    for (auto& [_, deviceHandle] : deviceHandles_)
    {
        i2c_master_bus_rm_device(deviceHandle);
    }

    i2c_del_master_bus(busHandle_);
}

i2c_master_dev_handle_t Esp32I2cBus::getOrAddDevice(uint8_t address)
{
    auto it = deviceHandles_.find(address);
    if (it != deviceHandles_.end())
    {
        return it->second;
    }

    i2c_device_config_t deviceConfig{};
    deviceConfig.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    deviceConfig.device_address = address;
    deviceConfig.scl_speed_hz = 100000;

    i2c_master_dev_handle_t deviceHandle{};
    i2c_master_bus_add_device(busHandle_, &deviceConfig, &deviceHandle);

    deviceHandles_.insert({address, deviceHandle});
    return deviceHandle;
}

bool Esp32I2cBus::write(uint8_t address, const uint8_t* data, size_t length)
{
    return i2c_master_transmit(getOrAddDevice(address), data, length, kI2cTimeoutMs) == ESP_OK;
}

bool Esp32I2cBus::read(uint8_t address, uint8_t* buffer, size_t length)
{
    return i2c_master_receive(getOrAddDevice(address), buffer, length, kI2cTimeoutMs) == ESP_OK;
}
