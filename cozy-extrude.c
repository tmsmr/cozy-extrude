#include "pico/stdlib.h"
#include <stdio.h>
#include <fan.h>
#include <bme280.h>

int main() {
    stdio_init_all();

    bme280_t bme280 = bme280_init(0, 18, 19, 16, 17);
    init_fan(2, 15);

    uint8_t dc = FAN_MIN_DC;
    bool rising = true;

    while (true) {
        set_fan_dc(dc);
        sleep_ms(1000);
        printf("\r----> %.2f °C, DC: %d, RPM: %d <----", bm280_read_temp(&bme280), dc, get_fan_rpm());
        if (dc >= 100) rising = false;
        if (dc <= FAN_MIN_DC) rising = true;
        if (rising) dc += 1;
        else dc -= 1;
    }
}