#include "cmds.h"
#include <stdio.h>

#define SERIAL_TIMEOUT_US 1000

void poll_next_command(serial_command *cmd) {
    uint8_t i, sum;
    int val;
    retry_serial_rx:
    // wait for preamble
    while (getchar_timeout_us(0) != SERIAL_COMMAND_PRE);
    // read cmd
    val = getchar_timeout_us(SERIAL_TIMEOUT_US);
    if (val == PICO_ERROR_TIMEOUT) goto retry_serial_rx;
    cmd->cmd = (uint8_t) val;
    sum = cmd->cmd;
    // read payload length
    val = getchar_timeout_us(SERIAL_TIMEOUT_US);
    if (val == PICO_ERROR_TIMEOUT) goto retry_serial_rx;
    cmd->payload_len = (uint8_t) val;
    sum ^= cmd->payload_len;
    // avoid payload overflow
    if (cmd->payload_len > CMD_MAX_PAYLOAD_LEN) goto retry_serial_rx;
    // read payload
    for (i = 0; i < cmd->payload_len; i++) {
        val = getchar_timeout_us(SERIAL_TIMEOUT_US);
        if (val == PICO_ERROR_TIMEOUT) goto retry_serial_rx;
        cmd->payload[i] = (uint8_t) val;
        sum ^= cmd->payload[i];
    }
    // read checksum
    val = getchar_timeout_us(SERIAL_TIMEOUT_US);
    if (val == PICO_ERROR_TIMEOUT) goto retry_serial_rx;
    // verify checksum
    if (((uint8_t) val) != sum) goto retry_serial_rx;
}

void send_command_response(serial_command *cmd) {
    uint8_t i, sum = cmd->cmd ^ cmd->payload_len;
    putchar(SERIAL_COMMAND_PRE);
    putchar(cmd->cmd);
    putchar(cmd->payload_len);
    for (i = 0; i < cmd->payload_len; i++) {
        putchar(cmd->payload[i]);
        sum ^= cmd->payload[i];
    }
    putchar(sum);
}
