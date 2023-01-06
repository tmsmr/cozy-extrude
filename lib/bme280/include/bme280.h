#ifndef COZY_EXTRUDE_BME280_H
#define COZY_EXTRUDE_BME280_H

#include "pico/stdlib.h"
#include "hardware/spi.h"

typedef struct {
    spi_inst_t *spi;
    uint csn_gpio;

    uint16_t dig_T1;
    int16_t dig_T2;
    int16_t dig_T3;
} bme280_t;

/*
 * e.g. bme280_t bme280 = bme280_init(0, 18, 19, 16, 17);
 *
 * spi0 (spi_default)
 *
 * GPIO 18, SPI0 SCK (PICO_DEFAULT_SPI_SCK_PIN) <-> SCL
 * GPIO 19, SPI0 TX (PICO_DEFAULT_SPI_TX_PIN)   <-> SDA
 * GPIO 16, SPI0 RX (PICO_DEFAULT_SPI_RX_PIN)   <-> SDO
 * GPIO 17, SPI0 CSn (PICO_DEFAULT_SPI_CSN_PIN) <-> CSN
 *
 * 3v3                                          <-> VCC
 * GND                                          <-> GND
 */

bme280_t bme280_init(uint8_t spi_num, uint sck_gpio, uint tx_gpio, uint rx_gpio, uint csn_gpio);

float bm280_read_temp(bme280_t *bme280);

#endif
