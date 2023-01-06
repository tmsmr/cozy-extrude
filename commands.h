#ifndef COZY_EXTRUDE_COMMANDS_H
#define COZY_EXTRUDE_COMMANDS_H

#define CMD_MAX_LEN 7

#define COMMAND_GET_TEMP 0x1A
#define COMMAND_GET_FAN_DC 0x2A
#define COMMAND_GET_FAN_RPM 0x3A
#define COMMAND_SET_TGT_TEMP 0x1B

#include "pico/stdlib.h"

void flush_serial_rx();

void poll_serial_command(uint8_t buf[]);

void send_byte_response(uint8_t cmd, uint8_t val);

void send_word_response(uint8_t cmd, uint16_t val);

void send_dword_response(uint8_t cmd, uint32_t val);

#endif
