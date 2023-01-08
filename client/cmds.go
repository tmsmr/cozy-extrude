package main

const (
	SerialCommandPre = '|'
	CmdMaxPayloadLen = 8

	CmdGetTemp    = 0x1A
	CmdGetFanDc   = 0x2A
	CmdGetFanRpm  = 0x3A
	CmdGetTgtTemp = 0x4A
	CmdSetTgtTemp = 0x5A
)

type SerialCommand struct {
	cmd        byte
	payloadLen byte
	payload    [CmdMaxPayloadLen]byte
}
