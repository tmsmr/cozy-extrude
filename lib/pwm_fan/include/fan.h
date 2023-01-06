#ifndef COZY_EXTRUDE_FAN_H
#define COZY_EXTRUDE_FAN_H

#include "pico/stdlib.h"

#define FAN_MIN_DC 20

/*
 * e.g. init_fan(2, 15);
 */

void init_fan(uint pwm_gpio, uint tach_gpio);

void set_fan_dc(uint8_t dc);

uint16_t get_fan_rpm();

#endif
