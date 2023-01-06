#include <fan.h>
#include "hardware/pwm.h"
#include "hardware/clocks.h"

#define FAN_PWM_FREQ 25000
#define FAN_PWM_RES 100
#define FAN_RPM_CALC_FREQ 0.5

uint _pwm_gpio;
uint _tach_gpio;

volatile uint16_t _fan_tach;
volatile uint16_t _fan_rpm;
struct repeating_timer _fan_rpm_rt;

void _fan_tach_isr(uint _1, uint32_t _2) {
    _fan_tach++;
}

bool _fan_rpm_calc_isr(struct repeating_timer *_1) {
    _fan_rpm = _fan_tach * FAN_RPM_CALC_FREQ * 30;
    _fan_tach = 0;
    return true;
}

void init_fan(uint pwm_gpio, uint tach_gpio) {
    _pwm_gpio = pwm_gpio;
    _tach_gpio = tach_gpio;
    uint32_t sys_clk_hz = clock_get_hz(clk_sys);
    uint32_t pwm_div = sys_clk_hz / FAN_PWM_FREQ / FAN_PWM_RES;
    gpio_set_function(_pwm_gpio, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(_pwm_gpio);
    pwm_set_clkdiv_int_frac(slice_num, pwm_div, 0);
    pwm_set_wrap(slice_num, FAN_PWM_RES - 1);
    pwm_set_enabled(slice_num, true);
    set_fan_dc(0);
    gpio_set_irq_enabled_with_callback(_tach_gpio, GPIO_IRQ_EDGE_RISE, true, &_fan_tach_isr);
    int32_t rpm_calc_delay = (int32_t) (1000 / FAN_RPM_CALC_FREQ);
    add_repeating_timer_ms(-rpm_calc_delay, _fan_rpm_calc_isr, NULL, &_fan_rpm_rt);
}

void set_fan_dc(uint8_t dc) {
    if (dc < FAN_MIN_DC) dc = 0;
    uint slice_num = pwm_gpio_to_slice_num(_pwm_gpio);
    pwm_set_chan_level(slice_num, PWM_CHAN_A, dc);
}

uint16_t get_fan_rpm() {
    return _fan_rpm;
}
