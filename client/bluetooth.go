package client

import (
	"fmt"
	"time"

	"github.com/paypal/gatt"
)

const bluetoothPacketSize = 20

var uartServiceID = gatt.MustParseUUID("6e400001-b5a3-f393-e0a9-e50e24dcca9e")
var uartServiceRXCharID = gatt.MustParseUUID("6e400002-b5a3-f393-e0a9-e50e24dcca9e")
var uartServiceTXCharID = gatt.MustParseUUID("6e400003-b5a3-f393-e0a9-e50e24dcca9e")

type bluetoothConnection struct {
	Periph         gatt.Peripheral
	Characteristic *gatt.Characteristic
}

type Bluetooth struct {
	in         <-chan [][]byte
	out        chan<- []byte
	connection chan bluetoothConnection
}

func NewBluetooth(in <-chan [][]byte, out chan<- []byte) *Bluetooth {
	return &Bluetooth{in: in, out: out, connection: make(chan bluetoothConnection)}
}

func (this *Bluetooth) Start() {
	go this.write()

	for {
		device, err := gatt.NewDevice(
			gatt.LnxMaxConnections(1),
			gatt.LnxDeviceID(-1, false),
		)
		if err != nil {
			fmt.Printf("Failed to open bluetooth device: %s", err)
			time.Sleep(time.Second / 2)
			continue
		}

		device.Handle(
			gatt.PeripheralDiscovered(this.onDiscovered),
			gatt.PeripheralConnected(this.onConnected),
			gatt.PeripheralDisconnected(this.onDisconnected),
		)

		device.Init(this.onStateChanged)
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
					fmt.Printf("Cannot send message over bluetooth because it is larger than %d bytes\n",
						bluetoothPacketSize,
					)
					continue
				}
				for len(msg) > 0 && len(packet)+len(msg[0]) <= bluetoothPacketSize {
					packet = append(packet, msg[0]...)
					msg = msg[1:]
				}

				// TODO deal with call blocking on connection close
				err := connection.Periph.WriteCharacteristic(connection.Characteristic, packet, true)
				if err != nil {
					fmt.Println("Failed to send message over bluetooth:", err)
				}
			}
		}
	}
}

func (this *Bluetooth) read(c *gatt.Characteristic, b []byte, err error) {
	// Assume we receive the entire message at once (even though in theory we
	// could wait for more messages to come to satisfy the length byte)
	this.out <- b
}

func (this *Bluetooth) onStateChanged(device gatt.Device, state gatt.State) {
	switch state {
	case gatt.StatePoweredOn:
		device.Scan(nil, false)
	default:
		device.StopScanning()
	}
}

func (this *Bluetooth) onDiscovered(periph gatt.Peripheral, ad *gatt.Advertisement, rssi int) {
	if ad.LocalName == "UART" {
		periph.Device().StopScanning()
		periph.Device().Connect(periph)
	}
}

func (this *Bluetooth) onConnected(periph gatt.Peripheral, err error) {
	if err != nil {
		fmt.Println("Peripheral connected with error:", err)
		return
	}

	services, err := periph.DiscoverServices(nil)
	if err != nil {
		fmt.Println("Failed to discover services")
		return
	}

	for _, service := range services {

		if service.UUID().Equal(uartServiceID) {
			cs, err := periph.DiscoverCharacteristics(nil, service)
			if err != nil {
				fmt.Println("Error discovering characteristics:", err)
				return
			}

			rxFound, txFound := false, false
			for _, c := range cs {
				if c.UUID().Equal(uartServiceTXCharID) {
					txFound = true
					periph.DiscoverDescriptors(nil, c)
					periph.SetNotifyValue(c, this.read)

				} else if c.UUID().Equal(uartServiceRXCharID) {
					rxFound = true
					this.connection <- bluetoothConnection{periph, c}
				}
			}
			if !rxFound || !txFound {
				fmt.Println("Could not find all characteristics of bluetooth device")
				return
			}
			fmt.Println("Connected to copter on bluetooth")
		}
	}
}

func (this *Bluetooth) onDisconnected(periph gatt.Peripheral, err error) {
	fmt.Println("Disconnected from copter on bluetooth")
	periph.Device().Scan(nil, false)
}
