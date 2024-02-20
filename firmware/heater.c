#include "heater.h"

#define HEATER_GPIO 22

void init_heater() {
    gpio_init(HEATER_GPIO);
    gpio_set_dir(HEATER_GPIO, GPIO_OUT);
}

void set_heating(bool heating) {
    gpio_put(HEATER_GPIO, heating);
}
