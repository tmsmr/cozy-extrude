package main

import (
	"encoding/binary"
	"errors"
	"time"
)

const (
	SerialCommandPre = '|'
	CmdMaxPayloadLen = 8

	CmdGetTemp    = 0x1A
	CmdGetFanDc   = 0x2A
	CmdGetFanRpm  = 0x3A
	CmdGetTgtTemp = 0x4A
	CmdSetTgtTemp = 0x5A

	CmdResponseTimeout = 10 * time.Millisecond
)

var ErrCmdSend = errors.New("failed to send command")
var ErrCmdTimout = errors.New("timed out waiting for command response")

type SerialCommand struct {
	cmd        byte
	payloadLen byte
	payload    [CmdMaxPayloadLen]byte
}

func ReqTempBlocking(ce *CozyExtrudeConn) (float32, error) {
	if (ce.SendCommand(&SerialCommand{
		cmd:        CmdGetTemp,
		payloadLen: 0,
		payload:    [8]byte{},
	}) != nil) {
		return 0, ErrCmdSend
	}
	resp := ce.WaitResponseWithTimeout(CmdResponseTimeout)
	if resp == nil {
		return 0, ErrCmdTimout
	}
	temp := int(binary.BigEndian.Uint32(resp.payload[0:4]))
	return float32(temp) / 100.0, nil
}

func ReqTgtTempBlocking(ce *CozyExtrudeConn) (float32, error) {
	if (ce.SendCommand(&SerialCommand{
		cmd:        CmdGetTgtTemp,
		payloadLen: 0,
		payload:    [8]byte{},
	}) != nil) {
		return 0, ErrCmdSend
	}
	resp := ce.WaitResponseWithTimeout(CmdResponseTimeout)
	if resp == nil {
		return 0, ErrCmdTimout
	}
	tgtTemp := int(binary.BigEndian.Uint32(resp.payload[0:4]))
	return float32(tgtTemp) / 100.0, nil
}

func ReqFanDCBlocking(ce *CozyExtrudeConn) (uint8, error) {
	if (ce.SendCommand(&SerialCommand{
		cmd:        CmdGetFanDc,
		payloadLen: 0,
		payload:    [8]byte{},
	}) != nil) {
		return 0, ErrCmdSend
	}
	resp := ce.WaitResponseWithTimeout(CmdResponseTimeout)
	if resp == nil {
		return 0, ErrCmdTimout
	}
	return resp.payload[0], nil
}

func ReqFanRPMBlocking(ce *CozyExtrudeConn) (uint16, error) {
	if (ce.SendCommand(&SerialCommand{
		cmd:        CmdGetFanRpm,
		payloadLen: 0,
		payload:    [8]byte{},
	}) != nil) {
		return 0, ErrCmdSend
	}
	resp := ce.WaitResponseWithTimeout(CmdResponseTimeout)
	if resp == nil {
		return 0, ErrCmdTimout
	}
	return binary.BigEndian.Uint16(resp.payload[0:2]), nil
}

func ReqSetTgtTempBlocking(ce *CozyExtrudeConn, tgtTemp uint16) error {
	cmd := SerialCommand{
		cmd:        CmdSetTgtTemp,
		payloadLen: 4,
		payload:    [8]byte{},
	}
	buf := make([]byte, 4)
	binary.BigEndian.PutUint32(buf, uint32(tgtTemp))
	copy(cmd.payload[:], buf[:4])
	if ce.SendCommand(&cmd) != nil {
		return ErrCmdSend
	}
	resp := ce.WaitResponseWithTimeout(CmdResponseTimeout)
	if resp == nil {
		return ErrCmdTimout
	}
	return nil
}
