#ifndef COZY_EXTRUDE_CMDS_H
#define COZY_EXTRUDE_CMDS_H

#define CMD_MAX_PAYLOAD_LEN 8

#define CMD_GET_TEMP 0x1A
#define CMD_GET_FAN_DC 0x2A
#define CMD_GET_FAN_RPM 0x3A
#define CMD_GET_TGT_TEMP 0x4A
#define CMD_SET_TGT_TEMP 0x5A

#include "pico/stdlib.h"

typedef struct {
    uint8_t cmd;
    uint8_t payload_len;
    uint8_t payload[CMD_MAX_PAYLOAD_LEN];
} serial_command;

void poll_next_command(serial_command *cmd);

void send_command_response(serial_command *cmd);

#endif
