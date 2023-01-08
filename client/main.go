package main

import (
	"encoding/binary"
	"fmt"
	"log"
)

func main() {
	ce, err := NewCozyExtrude()
	if err != nil {
		log.Fatal(err)
	}
	defer ce.Close()
	go ce.StartRxPump()

	go func() {
		counter := 0
		for cmd := range ce.responses {
			switch cmd.cmd {
			case CmdGetTemp:
				temp := int(binary.BigEndian.Uint32(cmd.payload[0:4]))
				fmt.Printf("\r %.2f °C %d", float32(temp)/100, counter)
				counter++
				break
			default:
				break
			}
		}
	}()

	for {
		//time.Sleep(1 * time.Millisecond)
		ce.SendCommand(&SerialCommand{
			cmd:        CmdGetTemp,
			payloadLen: 0,
			payload:    [8]byte{},
		})
		/*cmd := ce.WaitCommandResponse()
		temp := int(binary.BigEndian.Uint32(cmd.payload[0:4]))
		fmt.Printf("\r %f °C", float32(temp)/100)*/
	}
}
