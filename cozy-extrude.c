#include "pico/stdlib.h"
#include <stdio.h>
#include "hardware/clocks.h"
#include "fan.h"
#include "config.h"

int main() {
    stdio_init_all();
    uint8_t dc = FAN_MIN_DC;
    init_fan();

    bool rising = true;

    while (true) {
        set_fan_dc(dc);
        sleep_ms(5000);
        printf("\r----> DC: %d, RPM: %d <----", dc, get_fan_rpm());
        if (dc >= 100) rising = false;
        if (dc <= FAN_MIN_DC) rising = true;
        if (rising) dc += 1;
        else dc -= 1;
    }
}