package client

import (
	"fmt"
	"time"

	"github.com/tarm/serial"
)

const (
	serialRetryPeriod = time.Second / 4
	serialTimeout     = time.Second / 10
)

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
			time.Sleep(serialRetryPeriod)
		}
	}()
}

func (this *Serial) startSession(portChan chan *serial.Port) {
	config := serial.Config{
		Name:        this.portName,
		Baud:        this.baud,
		ReadTimeout: serialTimeout,
	}
	port, err := serial.OpenPort(&config)
	if err != nil {
		return
	}
	defer port.Close()
	this.Connected <- true
	this.status <- fmt.Sprint("Serial connected on", this.portName)

	portChan <- port

	disconnect := func() {
		this.Connected <- false
		this.status <- fmt.Sprint("Serial disconnected on", this.portName)
	}

	buf := make([]byte, 256)
	for {
		t := time.Now()
		n, _ := port.Read(buf[:1])
		if n == 0 {
			if time.Since(t) < serialTimeout/2 {
				disconnect()
				return
			}
			continue
		}

		msgTarget := buf[1 : buf[0]+1]
		t = time.Now()
		n, _ = port.Read(msgTarget)
		if n < len(msgTarget) {
			if time.Since(t) < serialTimeout/2 {
				disconnect()
				return
			}
			continue
		}

		// Assume we've received the entire message at this point
		outBuf := make([]byte, len(msgTarget)+1)
		copy(outBuf, buf[:len(msgTarget)+1])
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
