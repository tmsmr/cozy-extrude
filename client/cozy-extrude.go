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
}

type statusOutput struct {
	Temperature       float32 `json:"temperature"`
	TargetTemperature float32 `json:"targetTemperature"`
	FanDutyCycle      uint8   `json:"fanDutyCycle"`
	FanRPM            uint16  `json:"fanRPM"`
}

func fatalOnErr(err error) {
	if err != nil {
		log.Fatal(err)
	}
}

func printStatus(ce *CozyExtrudeConn) {
	out := statusOutput{}
	temp, err := ReqTempBlocking(ce)
	fatalOnErr(err)
	out.Temperature = temp
	tgtTemp, err := ReqTgtTempBlocking(ce)
	fatalOnErr(err)
	out.TargetTemperature = tgtTemp
	fanDC, err := ReqFanDCBlocking(ce)
	fatalOnErr(err)
	out.FanDutyCycle = fanDC
	fanRPM, err := ReqFanRPMBlocking(ce)
	fatalOnErr(err)
	out.FanRPM = fanRPM
	jsonOut, err := json.MarshalIndent(&out, "", "  ")
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
	}
}
