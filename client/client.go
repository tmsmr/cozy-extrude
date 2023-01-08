package main

import (
	"encoding/binary"
	"fmt"
	"log"
	"time"
)

func main() {
	ce, err := NewCozyExtrudeConn()
	if err != nil {
		log.Fatal(err)
	}
	defer ce.Close()
	go ce.RxPump()

	var temp int
	var tgtTemp int
	var fanDc uint8
	var fanRpm uint16

	go func() {
		for cmd := range ce.responses {
			switch cmd.cmd {
			case CmdGetTemp:
				temp = int(binary.BigEndian.Uint32(cmd.payload[0:4]))
				break
			case CmdGetFanDc:
				fanDc = cmd.payload[0]
				break
			case CmdGetFanRpm:
				fanRpm = binary.BigEndian.Uint16(cmd.payload[0:2])
				break
			case CmdGetTgtTemp:
				tgtTemp = int(binary.BigEndian.Uint32(cmd.payload[0:4]))
				break
			case CmdSetTgtTemp:
				break
			default:
				break
			}
		}
	}()

	var rqTgtTemp = 250
	go func() {
		for {
			time.Sleep(20 * time.Millisecond)
			cmd := SerialCommand{
				cmd:        CmdSetTgtTemp,
				payloadLen: 4,
				payload:    [8]byte{},
			}
			buf := make([]byte, 4)
			binary.BigEndian.PutUint32(buf, uint32(rqTgtTemp))
			copy(cmd.payload[:], buf[:4])
			ce.SendCommand(&cmd)
			rqTgtTemp++
		}
	}()

	for {
		time.Sleep(10 * time.Millisecond)
		ce.SendCommand(&SerialCommand{
			cmd:        CmdGetTemp,
			payloadLen: 0,
			payload:    [8]byte{},
		})
		ce.SendCommand(&SerialCommand{
			cmd:        CmdGetFanDc,
			payloadLen: 0,
			payload:    [8]byte{},
		})
		ce.SendCommand(&SerialCommand{
			cmd:        CmdGetFanRpm,
			payloadLen: 0,
			payload:    [8]byte{},
		})
		ce.SendCommand(&SerialCommand{
			cmd:        CmdGetTgtTemp,
			payloadLen: 0,
			payload:    [8]byte{},
		})
		fmt.Printf("\r%.2f/%.2f °C, %d RPM (%d)", float32(temp)/100, float32(tgtTemp)/100, fanRpm, fanDc)
	}
}
