package client

import (
	"fmt"
	"io"
	"math"
	"time"

	"github.com/tarm/serial"
)

const handheldRetryPeriod = time.Millisecond * 500

type handheldVar struct {
	groupName, varName string
	value, lastValue   interface{}
}

func (this *handheldVar) Set(value interface{}) {
	this.lastValue, this.value = this.value, value
}

func (this *handheldVar) AddIfNew(vars []BoundVar) []BoundVar {
	if this.value != this.lastValue {
		return append(vars, MustBindVar(this.groupName, this.varName, this.value))
	} else {
		return vars
	}
}

type Handheld struct {
	port *serial.Port
	out  chan<- []BoundVar

	softKill, force, yaw, pitch, roll handheldVar
	vars                              []*handheldVar
}

func NewHandheld(out chan<- []BoundVar) *Handheld {
	this := new(Handheld)
	this.out = out

	this.softKill = handheldVar{groupName: "switches", varName: "softKill"}
	this.force = handheldVar{groupName: "desires", varName: "force"}
	this.yaw = handheldVar{groupName: "desires", varName: "yaw"}
	this.pitch = handheldVar{groupName: "desires", varName: "pitch"}
	this.roll = handheldVar{groupName: "desires", varName: "roll"}

	this.vars = []*handheldVar{
		&this.softKill,
		&this.force,
		&this.yaw,
		&this.pitch,
		&this.roll,
	}
	return this
}

func (this *Handheld) Start() {
	go func() {
		for {
			this.startSession()
			time.Sleep(handheldRetryPeriod)
		}
	}()
}

func (this *Handheld) startSession() {
	config := serial.Config{Name: "/dev/ttyUSB0", Baud: 38400}
	var err error
	if this.port, err = serial.OpenPort(&config); err != nil {
		return
	}
	defer this.port.Close()
	fmt.Println("Handheld connected")

	for {
		var softKill bool
		var leftX, leftY, rightX, rightY float64
		_, err = fmt.Fscanln(this.port, &softKill, &leftX, &leftY, &rightX, &rightY)
		if err == io.EOF {
			fmt.Println("Handeld disconnected")
			return
		} else if err != nil {
			// We've probably read during a startup transient period
			continue
		}

		this.outputVars(softKill, leftX, leftY, rightX, rightY)
	}
}

func (this *Handheld) outputVars(softKill bool, leftX, leftY, rightX, rightY float64) {
	const maxInput = 1023
	const minForce, maxForce = 0.05, 1
	const minTilt, maxTilt = 1, 5

	force := (leftY - maxInput/2) / (maxInput / 2)
	force = math.Min(force, maxForce)
	yaw := (leftX - maxInput/2) / maxInput * 360
	pitch := (rightY - maxInput/2) / (maxInput / 2) * maxTilt
	roll := -(rightX - maxInput/2) / (maxInput / 2) * maxTilt

	this.softKill.Set(softKill)
	this.force.Set(toZero(force, minForce))
	this.yaw.Set(toZero(yaw, minTilt))
	this.pitch.Set(toZero(pitch, minTilt))
	this.roll.Set(toZero(roll, minTilt))

	var outVars []BoundVar
	for _, v := range this.vars {
		outVars = v.AddIfNew(outVars)
	}
	if len(outVars) > 0 {
		this.out <- outVars
	}
}

func toZero(v, min float64) float64 {
	if math.Abs(v) < min {
		return 0
	}
	return v
}
