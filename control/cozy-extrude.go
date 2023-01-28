package main

import (
	"encoding/json"
	"github.com/alecthomas/kong"
	"log"
)

var cli struct {
	Status struct {
	} `cmd:"status"`
	Set struct {
		Temp uint16 `arg`
	} `cmd:"set"`
	Tui struct {
	} `cmd:"tui"`
	Serve struct {
	} `cmd:"serve"`
}

type statusOutput struct {
	Temperature  float32 `json:"temperature"`
	Target       float32 `json:"target"`
	FanDutyCycle uint8   `json:"fanDutyCycle"`
	FanRPM       uint16  `json:"fanRPM"`
}

func fatalOnErr(err error) {
	if err != nil {
		log.Fatal(err)
	}
}

func readStatus(ce *CozyExtrudeConn) (*statusOutput, error) {
	status := statusOutput{}
	temp, err := ReqTempBlocking(ce)
	if err != nil {
		return nil, err
	}
	status.Temperature = temp
	tgtTemp, err := ReqTgtTempBlocking(ce)
	if err != nil {
		return nil, err
	}
	status.Target = tgtTemp
	fanDC, err := ReqFanDCBlocking(ce)
	if err != nil {
		return nil, err
	}
	status.FanDutyCycle = fanDC
	fanRPM, err := ReqFanRPMBlocking(ce)
	if err != nil {
		return nil, err
	}
	status.FanRPM = fanRPM
	return &status, nil
}

func printStatus(ce *CozyExtrudeConn) {
	status, err := readStatus(ce)
	fatalOnErr(err)
	jsonOut, err := json.MarshalIndent(&status, "", "  ")
	fatalOnErr(err)
	println(string(jsonOut))
}

func main() {
	cliCtx := kong.Parse(&cli)

	ce, err := NewCozyExtrudeConn()
	if err != nil {
		log.Fatal(err)
	}
	defer ce.Close()

	switch cliCtx.Command() {
	case "status":
		printStatus(ce)
		break
	case "set <temp>":
		fatalOnErr(ReqSetTgtTempBlocking(ce, cli.Set.Temp*100))
		break
	case "tui":
		break
	case "serve":
		CozyExtrudeHTTP(ce)
	}
}
