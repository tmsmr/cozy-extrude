package main

import (
	"github.com/kelseyhightower/envconfig"
	"github.com/tarm/serial"
	"log"
	"sync"
)

type SerialEnvSpec struct {
	TTYDevice   string `envconfig:"COZY_EXTRUDE_TTY" default:"/dev/ttyACM0"`
	TTYBaudrate uint32 `envconfig:"COZY_EXTRUDE_BAUDRATE" default:"115200"`
}

var serialEnv SerialEnvSpec

func init() {
	if err := envconfig.Process("", &serialEnv); err != nil {
		log.Fatal(err)
	}
}

type CozyExtrude struct {
	conn      *serial.Port
	txMu      sync.Mutex
	responses chan SerialCommand
}

func NewCozyExtrude() (*CozyExtrude, error) {
	ce := CozyExtrude{
		txMu:      sync.Mutex{},
		responses: make(chan SerialCommand),
	}
	err := ce.Open()
	if err != nil {
		return nil, err
	}
	return &ce, nil
}

func (ce *CozyExtrude) Open() error {
	conn, err := serial.OpenPort(&serial.Config{
		Name: serialEnv.TTYDevice,
		Baud: int(serialEnv.TTYBaudrate),
	})
	if err != nil {
		return err
	}
	ce.conn = conn
	return nil
}

func (ce *CozyExtrude) Close() {
	close(ce.responses)
	_ = ce.conn.Close()
}

func (ce *CozyExtrude) StartRxPump() {
	for {
		resp := ce.WaitCommandResponse()
		ce.responses <- resp
	}
}

func (ce *CozyExtrude) WaitCommandResponse() SerialCommand {
	data := make([]byte, 1)
	var err error
PollSerialRx:
	// wait for preamble
	for {
		if _, err = ce.conn.Read(data); err == nil && data[0] == SerialCommandPre {
			break
		}
	}
	cmd := SerialCommand{}
	// read cmd
	if _, err = ce.conn.Read(data); err != nil {
		goto PollSerialRx
	}
	cmd.cmd = data[0]
	checksum := cmd.cmd
	// read payload length
	if _, err = ce.conn.Read(data); err != nil {
		goto PollSerialRx
	}
	cmd.payloadLen = data[0]
	checksum ^= cmd.payloadLen
	// avoid payload overflow
	if cmd.payloadLen > CmdMaxPayloadLen {
		goto PollSerialRx
	}
	// read payload
	for i := byte(0); i < cmd.payloadLen; i++ {
		if _, err = ce.conn.Read(data); err != nil {
			goto PollSerialRx
		}
		cmd.payload[i] = data[0]
		checksum ^= cmd.payload[i]
	}
	// read checksum
	if _, err = ce.conn.Read(data); err != nil {
		goto PollSerialRx
	}
	// verify checksum
	if data[0] != checksum {
		goto PollSerialRx
	}
	return cmd
}

func (ce *CozyExtrude) SendCommand(cmd *SerialCommand) error {
	data := make([]byte, 0)
	data = append(data, SerialCommandPre, cmd.cmd, cmd.payloadLen)
	checksum := cmd.cmd ^ cmd.payloadLen
	for i := byte(0); i < cmd.payloadLen; i++ {
		data = append(data, cmd.payload[i])
		checksum ^= cmd.payload[i]
	}
	data = append(data, checksum)
	ce.txMu.Lock()
	_, err := ce.conn.Write(data)
	ce.txMu.Unlock()
	return err
}
