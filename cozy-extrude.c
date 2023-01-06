#include "pico/stdlib.h"
#include <stdio.h>
#include <fan.h>
#include <bme280.h>
#include "commands.h"

#define BME280_SPI_NUM 0
#define BME280_SPI_SCK 18
#define BME280_SPI_TX 19
#define BME280_SPI_RX 16
#define BME280_SPI_CSN 17

#define FAN_PWM_GPIO 2
#define FAN_TACH_GPIO 15

int main() {
    stdio_init_all();

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

    uint8_t fan_dc = FAN_MIN_DC;

    flush_serial_rx();
    uint8_t cmd[CMD_MAX_LEN];
    while (true) {
        poll_serial_command(cmd);
        switch (cmd[0]) {
            case COMMAND_GET_TEMP:
                send_dword_response(COMMAND_GET_TEMP, bm280_read_temp(&bme280));
                break;
            case COMMAND_GET_FAN_DC:
                send_byte_response(COMMAND_GET_FAN_DC, fan_dc);
                break;
            case COMMAND_GET_FAN_RPM:
                send_word_response(COMMAND_GET_FAN_RPM, get_fan_rpm());
                break;
            case COMMAND_SET_TGT_TEMP:
                break;
            default:
                break;
        }
    }
}
