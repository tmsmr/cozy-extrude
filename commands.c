#include "commands.h"
#include <stdio.h>

#define SERIAL_TIMEOUT_US 1000
#define SERIAL_COMMAND_DELIMIT '|'

void flush_serial_rx() {
    while (getchar_timeout_us(SERIAL_TIMEOUT_US) != PICO_ERROR_TIMEOUT);
}

void poll_serial_command(uint8_t buf[]) {
    int8_t i, j, sum;
    int val;
    poll_serial_rx:
    for (i = 0; i < CMD_MAX_LEN; i++) {
        val = getchar_timeout_us(SERIAL_TIMEOUT_US);
        switch (val) {
            case PICO_ERROR_TIMEOUT:
                goto poll_serial_rx;
            case SERIAL_COMMAND_DELIMIT:
                if (i < 2) goto poll_serial_rx;
                sum = buf[0];
                for (j = 1; j < i - 1; j++) sum ^= buf[j];
                if (sum != buf[i - 1]) goto poll_serial_rx;
                return;
            default:
                buf[i] = val;
        }
    }
    goto poll_serial_rx;
}

void send_byte_response(uint8_t cmd, uint8_t val) {
    putchar(cmd);
    putchar(val);
    putchar(cmd ^ val);
    putchar(SERIAL_COMMAND_DELIMIT);
}

void send_word_response(uint8_t cmd, uint16_t val) {
    uint8_t sum = cmd;
    putchar(cmd);
    for (int i = 1; i >= 0; i--) {
        uint8_t _byte = (uint8_t) (val >> (8 * i));
        sum ^= _byte;
        putchar(_byte);
    }
    putchar(sum);
    putchar(SERIAL_COMMAND_DELIMIT);
}

void send_dword_response(uint8_t cmd, uint32_t val) {
    uint8_t sum = cmd;
    putchar(cmd);
    for (int i = 3; i >= 0; i--) {
        uint8_t _byte = (uint8_t) (val >> (8 * i));
        sum ^= _byte;
        putchar(_byte);
    }
    putchar(sum);
    putchar(SERIAL_COMMAND_DELIMIT);
}
