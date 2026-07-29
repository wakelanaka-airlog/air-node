#include "Esp32Delay.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

void Esp32Delay::delayMs(uint32_t milliseconds)
{
    vTaskDelay(pdMS_TO_TICKS(milliseconds));
}
