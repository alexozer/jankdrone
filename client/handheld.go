package client

import (
	"fmt"
	"io"
	"math"
	"time"

	"github.com/tarm/serial"
)

const handheldRetryPeriod = time.Millisecond * 500

type Handheld struct {
	port *serial.Port
	out  chan<- []Var

	lastForce float64
	lastYaw   float64
	lastPitch float64
	lastRoll  float64
}

func NewHandheld(out chan<- []Var) *Handheld {
	return &Handheld{out: out}
}

func (this *Handheld) Start() {
	for {
		this.startSession()
		time.Sleep(handheldRetryPeriod)
	}
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
		var leftX, leftY, rightX, rightY int
		_, err = fmt.Fscanln(this.port, &leftX, &leftY, &rightX, &rightY)
		if err == io.EOF {
			fmt.Println("Handeld disconnected")
			return
		} else if err != nil {
			// We've probably read during a startup transient period
			continue
		}

		this.outputVars(leftX, leftY, rightX, rightY)
	}
}

func (this *Handheld) outputVars(leftX, leftY, rightX, rightY int) {
	const maxInput = 1023
	const minForce, maxForce = 0.05, 1
	const minTilt, maxTilt = 1, 5

	force := float64(leftY-maxInput/2) / (maxInput / 2)
	yaw := float64(leftX-maxInput/2) / maxInput * 360
	pitch := float64(rightY-maxInput/2) / (maxInput / 2) * maxTilt
	roll := float64(rightX-maxInput/2) / (maxInput / 2) * maxTilt

	force = math.Min(force, maxForce)
	force = toZero(force, minForce)
	yaw = toZero(yaw, minTilt)
	pitch = toZero(pitch, minTilt)
	roll = toZero(roll, minTilt)

	var outBuf []Var
	if force != this.lastForce {
		outBuf = append(outBuf, MustBindVar("controller", "verticalForce", force))
		this.lastForce = force
	}
	if yaw != this.lastYaw {
		outBuf = append(outBuf, MustBindVar("controller", "yawDesire", yaw))
		this.lastYaw = yaw
	}
	if pitch != this.lastPitch {
		outBuf = append(outBuf, MustBindVar("controller", "pitchDesire", pitch))
		this.lastPitch = pitch
	}
	if roll != this.lastRoll {
		outBuf = append(outBuf, MustBindVar("controller", "rollDesire", roll))
		this.lastRoll = roll
	}

	if len(outBuf) > 0 {
		this.out <- outBuf
	}
}

func toZero(v, min float64) float64 {
	if math.Abs(v) < min {
		return 0
	}
	return v
}
