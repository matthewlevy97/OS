#pragma once

#include <drivers/drivers.h>

#define PIT_HERTZ                      (3579545.0 / 3)
#define PIT_FREQUENCY                  (1000)

#define PIT_CHANNEL0                   (0x40)
#define PIT_CHANNEL1                   (0x41)
#define PIT_CHANNEL2                   (0x42)
#define PIT_COMMAND                    (0x43)

struct pit_command_value {
    uint8_t bcd_mode       : 1;
    uint8_t operating_mode : 3;
    uint8_t access_mode    : 2;
    uint8_t channel        : 2;
} __packed;

bool pit_init();
bool pit_destroy();