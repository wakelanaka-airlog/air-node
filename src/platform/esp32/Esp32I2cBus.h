#pragma once

#include <cstddef>
#include <cstdint>
#include <map>

#include "driver/i2c_master.h"

#include "I2cBus.h"
#include "Logger.h"

class Esp32I2cBus : public I2cBus
{
public:
    Esp32I2cBus(i2c_port_num_t port, gpio_num_t sdaPin, gpio_num_t sclPin, Logger& logger);
    ~Esp32I2cBus() override;

    Esp32I2cBus(const Esp32I2cBus&) = delete;
    Esp32I2cBus& operator=(const Esp32I2cBus&) = delete;

    bool write(uint8_t address, const uint8_t* data, size_t length) override;
    bool read(uint8_t address, uint8_t* buffer, size_t length) override;

private:
    // Returns nullptr if the device could not be added (e.g. the bus itself
    // failed to initialize, or the device-add call failed).
    i2c_master_dev_handle_t getOrAddDevice(uint8_t address);

    Logger& logger_;
    i2c_master_bus_handle_t busHandle_ = nullptr;
    std::map<uint8_t, i2c_master_dev_handle_t> deviceHandles_;
};
