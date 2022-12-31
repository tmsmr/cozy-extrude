#ifndef COZY_EXTRUDE_FAN_H
#define COZY_EXTRUDE_FAN_H

#include "pico/stdlib.h"

void init_fan();

void set_fan_dc(uint8_t dc);

uint16_t get_fan_rpm();

#endif
