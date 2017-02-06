package client

import (
	"context"
	"fmt"
	"time"

	"github.com/currantlabs/ble"
	"github.com/currantlabs/ble/linux"
)

const bluetoothPacketSize = 20

var uartServiceID = ble.MustParse("6e400001-b5a3-f393-e0a9-e50e24dcca9e")
var uartServiceRXCharID = ble.MustParse("6e400002-b5a3-f393-e0a9-e50e24dcca9e")
var uartServiceTXCharID = ble.MustParse("6e400003-b5a3-f393-e0a9-e50e24dcca9e")

type bluetoothConnection struct {
	Client ble.Client
	Char   *ble.Characteristic
}

type Bluetooth struct {
	in         <-chan [][]byte
	out        chan<- []byte
	status     chan<- string
	connection chan bluetoothConnection
}

func NewBluetooth(in <-chan [][]byte, out chan<- []byte, status chan<- string) *Bluetooth {
	return &Bluetooth{
		in: in, out: out,
		status: status, connection: make(chan bluetoothConnection),
	}
}

func (this *Bluetooth) Start() {
	go this.write()

	device, err := linux.NewDevice()
	if err != nil {
		this.status <- fmt.Sprint("Failed to open bluetooth device:", err)
		return
	}
	ble.SetDefaultDevice(device)

	filter := func(a ble.Advertisement) bool {
		return a.LocalName() == "UART"
	}

	ctx := ble.WithSigHandler(context.WithTimeout(context.Background(), time.Second*5))
	client, err := ble.Connect(ctx, filter)
	if err != nil {
		this.status <- fmt.Sprint("Can't connect:", err)
		return
	}

	go func() {
		<-client.Disconnected()
		this.status <- "Bluetooth disconnected, attempting reconnect"
		device.Stop()
		this.Start()
	}()

	this.explore(client)
	this.status <- "Connected to bluetooth"
}

func (this *Bluetooth) explore(client ble.Client) {
	profile, err := client.DiscoverProfile(true)
	if err != nil {
		this.status <- fmt.Sprint("Failed to discover profile:", err)
		return
	}

	for _, service := range profile.Services {
		if !service.UUID.Equal(uartServiceID) {
			continue
		}

		rxFound, txFound := false, false
		for _, char := range service.Characteristics {
			if char.UUID.Equal(uartServiceRXCharID) {
				rxFound = true
				this.connection <- bluetoothConnection{client, char}

			} else if char.UUID.Equal(uartServiceTXCharID) {
				txFound = true
				if (char.Property & ble.CharNotify) == 0 {
					this.status <- "Bluetooth: TX characteristic does not have notify property"
					return
				}

				// Assume we receive entire message in single packet
				write := func(req []byte) { this.out <- req }
				if err = client.Subscribe(char, false, write); err != nil {
					this.status <- fmt.Sprint("Failed to subscribe:", err)
					return
				}
			}
		}
		if !rxFound || !txFound {
			this.status <- "Failed to discover all bluetooth characteristics"
			return
		}

		break
	}
}

func (this *Bluetooth) write() {
	var connection bluetoothConnection
	gotFirstConnection := false
	for !gotFirstConnection {
		select {
		case connection = <-this.connection:
			gotFirstConnection = true
		case <-this.in:
		}
	}

	for {
		select {
		case connection = <-this.connection:
		case msg := <-this.in:
			if len(msg) == 0 {
				continue
			}
			for len(msg) > 0 {
				// Init packet with first msg slice because it's most likely the entire packet
				packet := msg[0]
				msg = msg[1:]
				if len(packet) > bluetoothPacketSize {
					this.status <- fmt.Sprintf("Cannot send message over bluetooth because it is larger than %d bytes\n",
						bluetoothPacketSize,
					)
					continue
				}
				for len(msg) > 0 && len(packet)+len(msg[0]) <= bluetoothPacketSize {
					packet = append(packet, msg[0]...)
					msg = msg[1:]
				}

				// TODO deal with call blocking on connection close
				err := connection.Client.WriteCharacteristic(connection.Char, packet, true)
				if err != nil {
					this.status <- fmt.Sprint("Failed to send message over bluetooth:", err)
				}
			}
		}
	}
}
