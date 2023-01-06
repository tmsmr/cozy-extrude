#include <bme280.h>
#include "hardware/spi.h"

// 1MHz
#define TARGET_SPI_FREQ 1000000

// bst-bme280-ds002.pdf: 6.3.2 SPI read
#define READ_CMD_BIT 0b10000000
// bst-bme280-ds002.pdf: 6.3.1 SPI write
#define WRITE_CMD_MASK 0b01111111

// bst-bme280-ds002.pdf: 4.2.2 Trimming parameter readout
#define REG_START_COMP 0x88
// bst-bme280-ds002.pdf: 5.4.3 Register 0xF2 “ctrl_hum”
#define REG_CTRL_HUM 0xF2
// bst-bme280-ds002.pdf: 5.4.5 Register 0xF4 “ctrl_meas”
#define REG_CTRL_MEAS 0xF4
// bst-bme280-ds002.pdf: 5.4.6 Register 0xF5 “config”
#define REG_CONFIG 0xF5

// bst-bme280-ds002.pdf: 5.4.7 Register 0xF7…0xF9 “press” (_msb, _lsb, _xlsb), 5.4.8 Register 0xFA…0xFC “temp” (_msb, _lsb, _xlsb)
#define REG_START_RESULTS 0xF7

static void bme280_read(bme280_t *bme280, uint8_t address, uint8_t *dst, size_t len) {
    address |= READ_CMD_BIT;
    gpio_put(bme280->csn_gpio, 0);
    spi_write_blocking(bme280->spi, &address, 1);
    spi_read_blocking(bme280->spi, 0, dst, len);
    gpio_put(bme280->csn_gpio, 1);
}

static void bme280_write(bme280_t *bme280, uint8_t address, uint8_t data) {
    uint8_t buf[2];
    buf[0] = address & WRITE_CMD_MASK;
    buf[1] = data;
    gpio_put(bme280->csn_gpio, 0);
    spi_write_blocking(bme280->spi, buf, 2);
    gpio_put(bme280->csn_gpio, 1);
}

bme280_t bme280_init(uint8_t spi_num, uint sck_gpio, uint tx_gpio, uint rx_gpio, uint csn_gpio) {
    bme280_t bme280;
    bme280.csn_gpio = csn_gpio;
    switch (spi_num) {
        case 0:
            bme280.spi = spi0;
            break;
        case 1:
            bme280.spi = spi1;
            break;
    }

    spi_init(bme280.spi, TARGET_SPI_FREQ);
    gpio_set_function(sck_gpio, GPIO_FUNC_SPI);
    gpio_set_function(tx_gpio, GPIO_FUNC_SPI);
    gpio_set_function(rx_gpio, GPIO_FUNC_SPI);

    gpio_init(bme280.csn_gpio);
    gpio_set_dir(bme280.csn_gpio, GPIO_OUT);
    gpio_put(bme280.csn_gpio, 1);

    // read temperature compensation parameters from NVM
    uint8_t comp[6];
    bme280_read(&bme280, REG_START_COMP, comp, 6);
    bme280.dig_T1 = comp[0] | (comp[1] << 8);
    bme280.dig_T2 = comp[2] | (comp[3] << 8);
    bme280.dig_T3 = comp[4] | (comp[5] << 8);

    // skip humidity measurement
    bme280_write(&bme280, REG_CTRL_HUM, 0);

    // normal mode
    uint8_t ctrl_meas = 0b11;
    // keep bits 4,3,2 at 000 to skip pressure measurement
    // set temperature oversampling to x4
    ctrl_meas |= (0b011 << 5);
    bme280_write(&bme280, REG_CTRL_MEAS, ctrl_meas);

    return bme280;
}

static int32_t compensate_temp(bme280_t *bme280, uint32_t raw) {
    // bst-bme280-ds002.pdf: 8.2 Pressure compensation in 32 bit fixed point
    int32_t var1, var2, T;
    var1 = ((((raw >> 3) - ((int32_t) bme280->dig_T1 << 1))) * ((int32_t) bme280->dig_T2)) >> 11;
    var2 = (((((raw >> 4) - ((int32_t) bme280->dig_T1)) * ((raw >> 4) - ((int32_t) bme280->dig_T1))) >> 12) *
            ((int32_t) bme280->dig_T3)) >> 14;
    T = ((var1 + var2) * 5 + 128) >> 8;
    return T;
}

float bm280_read_temp(bme280_t *bme280) {
    // bst-bme280-ds002.pdf: 4. Data readout
    // -> Data readout is done by starting a burst read from 0xF7 to 0xFC (temperature and pressure)
    uint8_t buf[6];
    bme280_read(bme280, REG_START_RESULTS, buf, 6);
    // bst-bme280-ds002.pdf: 5.4.8 Register 0xFA…0xFC “temp” (_msb, _lsb, _xlsb)
    // -> buf[3] / 0xFA: temp_msb[7:0], Contains the MSB part ut[19:12] of the raw temperature measurement output data.
    // -> buf[4] / 0xFB: temp_lsb[7:0], Contains the LSB part ut[11:4] of the raw temperature measurement output data.
    // -> buf[5] / 0xFC: temp_xlsb[3:0], Contains the XLSB part ut[3:0] of the raw temperature measurement output data. Content depend on pressure resolution.
    uint32_t raw = ((uint32_t) buf[3] << 12) | ((uint32_t) buf[4] << 4) | (buf[5] >> 4);
    return compensate_temp(bme280, raw) / 100.0;
}
