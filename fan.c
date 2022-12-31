#include "fan.h"
#include "config.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

volatile uint16_t _fan_tach = 0;
volatile uint16_t _fan_rpm = 0;
struct repeating_timer _fan_rpm_rt;

void _fan_tach_isr(uint _1, uint32_t _2) {
    _fan_tach++;
}

bool _fan_rpm_calc_isr(struct repeating_timer *_1) {
    _fan_rpm = _fan_tach * FAN_RPM_CALC_FREQ * 30;
    _fan_tach = 0;
    return true;
}

void init_fan() {
    uint32_t sys_clk_hz = clock_get_hz(clk_sys);
    uint32_t pwm_div = sys_clk_hz / FAN_PWM_FREQ / FAN_PWM_RES;
    gpio_set_function(FAN_PWM_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(FAN_PWM_PIN);
    pwm_set_clkdiv_int_frac(slice_num, pwm_div, 0);
    pwm_set_wrap(slice_num, FAN_PWM_RES - 1);
    pwm_set_enabled(slice_num, true);
    set_fan_dc(0);
    gpio_set_irq_enabled_with_callback(FAN_TACH_PIN, GPIO_IRQ_EDGE_RISE, true, &_fan_tach_isr);
    int32_t rpm_calc_delay = (int32_t) (1000 / FAN_RPM_CALC_FREQ);
    add_repeating_timer_ms(-rpm_calc_delay, _fan_rpm_calc_isr, NULL, &_fan_rpm_rt);
}

void set_fan_dc(uint8_t dc) {
    if (dc < FAN_MIN_DC) dc = 0;
    uint slice_num = pwm_gpio_to_slice_num(FAN_PWM_PIN);
    pwm_set_chan_level(slice_num, PWM_CHAN_A, dc);
}

uint16_t get_fan_rpm() {
    return _fan_rpm;
}
