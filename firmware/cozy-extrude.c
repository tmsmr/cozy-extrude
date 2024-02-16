#include "pico/stdlib.h"
#include <stdio.h>
#include <fan.h>
#include <bme280.h>
#include "cmds.h"

#define BME280_SPI_NUM 0
#define BME280_SPI_SCK 18
#define BME280_SPI_TX 19
#define BME280_SPI_RX 16
#define BME280_SPI_CSN 17

#define FAN_PWM_GPIO 2
#define FAN_TACH_GPIO 15

#define READ_TEMP_DELAY_MS 10

#define DEF_TARGET_TEMP 3500
#define FAN_CTRL_DELAY_MS 100

volatile int32_t temp = 0;
volatile int32_t tgt_temp = DEF_TARGET_TEMP;
volatile uint8_t fan_dc = 0;

struct repeating_timer _read_temp_rt;
struct repeating_timer _ctrl_fan_dc_rt;

bme280_t bme280;

bool _read_temp_isr(struct repeating_timer *_1) {
    temp = bm280_read_temp(&bme280);
    return true;
}

bool _ctrl_fan_dc_isr(struct repeating_timer *_1) {
    uint8_t dc = 0;
    if (temp > tgt_temp) dc = ((temp - tgt_temp) / 10) + FAN_MIN_DC;
    if (dc > 100) dc = 100;
    set_fan_dc(dc);
    fan_dc = dc;
    return true;
}

int main() {
    stdio_init_all();
    stdio_set_translate_crlf(&stdio_uart, false);
    stdio_set_translate_crlf(&stdio_usb, false);

    bme280 = bme280_init(
            BME280_SPI_NUM,
            BME280_SPI_SCK,
            BME280_SPI_TX,
            BME280_SPI_RX,
            BME280_SPI_CSN
    );

    init_fan(
            FAN_PWM_GPIO,
            FAN_TACH_GPIO
    );

    add_repeating_timer_ms(-READ_TEMP_DELAY_MS, _read_temp_isr, NULL, &_read_temp_rt);
    add_repeating_timer_ms(-FAN_CTRL_DELAY_MS, _ctrl_fan_dc_isr, NULL, &_ctrl_fan_dc_rt);

    serial_command scmd;
    uint16_t fan_rpm;
    int32_t rq_temp;

    while (true) {
        poll_next_command(&scmd);
        switch (scmd.cmd) {
            case CMD_GET_TEMP:
                scmd.payload_len = 4;
                scmd.payload[0] = (uint8_t) (temp >> 24);
                scmd.payload[1] = (uint8_t) (temp >> 16);
                scmd.payload[2] = (uint8_t) (temp >> 8);
                scmd.payload[3] = (uint8_t) temp;
                send_command_response(&scmd);
                break;
            case CMD_GET_FAN_DC:
                scmd.payload_len = 1;
                scmd.payload[0] = fan_dc;
                send_command_response(&scmd);
                break;
            case CMD_GET_FAN_RPM:
                fan_rpm = get_fan_rpm();
                scmd.payload_len = 2;
                scmd.payload[0] = (uint8_t) (fan_rpm >> 8);
                scmd.payload[1] = (uint8_t) fan_rpm;
                send_command_response(&scmd);
                break;
            case CMD_GET_TGT_TEMP:
                scmd.payload_len = 4;
                scmd.payload[0] = (uint8_t) (tgt_temp >> 24);
                scmd.payload[1] = (uint8_t) (tgt_temp >> 16);
                scmd.payload[2] = (uint8_t) (tgt_temp >> 8);
                scmd.payload[3] = (uint8_t) tgt_temp;
                send_command_response(&scmd);
                break;
            case CMD_SET_TGT_TEMP:
                tgt_temp =
                        (scmd.payload[0] << 24)
                        | (scmd.payload[1] << 16)
                        | (scmd.payload[2] << 8)
                        | scmd.payload[3];
                scmd.payload_len = 0;
                send_command_response(&scmd);
                break;
            default:
                break;
        }
    }
}
