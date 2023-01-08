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

#define DEF_TARGET_TEMP 20

int main() {
    stdio_init_all();
    stdio_set_translate_crlf(&stdio_uart, false);
    stdio_set_translate_crlf(&stdio_usb, false);

    bme280_t bme280 = bme280_init(
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

    serial_command scmd;
    int32_t temp = 0;
    uint8_t fan_dc = 0;
    uint16_t fan_rpm;
    volatile uint8_t tgt_temp = DEF_TARGET_TEMP;

    while (true) {
        poll_next_command(&scmd);
        switch (scmd.cmd) {
            case CMD_GET_TEMP:
                temp = bm280_read_temp(&bme280);
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
                scmd.payload_len = 1;
                scmd.payload[0] = tgt_temp;
                send_command_response(&scmd);
                break;
            case CMD_SET_TGT_TEMP:
                tgt_temp = scmd.payload[0];
                scmd.payload_len = 0;
                send_command_response(&scmd);
                break;
            default:
                break;
        }
    }
}
