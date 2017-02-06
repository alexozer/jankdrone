package client

import (
	"fmt"
	"time"

	"github.com/tarm/serial"
)

const handheldRetryPeriod = time.Millisecond * 250

type Serial struct {
	In        chan [][]byte
	Out       chan []byte
	Connected chan bool

	portName string
	baud     int
	status   chan string
}

func NewSerial(portName string, baud int, status chan string) *Serial {
	return &Serial{
		In:        make(chan [][]byte),
		Out:       make(chan []byte),
		Connected: make(chan bool),

		portName: portName,
		baud:     baud,
		status:   status,
	}
}

func (this *Serial) Start() {
	portChan := make(chan *serial.Port)
	go this.write(portChan)

	go func() {
		for {
			this.startSession(portChan)
			time.Sleep(handheldRetryPeriod)
		}
	}()
}

func (this *Serial) startSession(portChan chan *serial.Port) {
	config := serial.Config{Name: this.portName, Baud: this.baud}
	port, err := serial.OpenPort(&config)
	if err != nil {
		return
	}
	defer port.Close()
	this.Connected <- true
	this.status <- fmt.Sprint("Serial connected on", this.portName)

	portChan <- port

	buf := make([]byte, 256)
	for {
		n, err := port.Read(buf)
		if err != nil {
			this.Connected <- false
			this.status <- fmt.Sprint("Serial disconnected on", this.portName)
			return
		}

		// Assume we get the whole message(s) at once
		outBuf := make([]byte, n)
		copy(outBuf, buf[:n])
		this.Out <- outBuf
	}
}

func (this *Serial) write(portChan chan *serial.Port) {
	port := <-portChan
	for {
		select {
		case port = <-portChan:
		case msg := <-this.In:
			for _, v := range msg {
				port.Write(v)
			}
		}
	}
}
