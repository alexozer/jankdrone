package client

import (
	"fmt"
	"time"

	"github.com/paypal/gatt"
)

var uartServiceID = gatt.MustParseUUID("6e400001-b5a3-f393-e0a9-e50e24dcca9e")
var uartServiceRXCharID = gatt.MustParseUUID("6e400002-b5a3-f393-e0a9-e50e24dcca9e")
var uartServiceTXCharID = gatt.MustParseUUID("6e400003-b5a3-f393-e0a9-e50e24dcca9e")

type bluetoothConnection struct {
	Periph         gatt.Peripheral
	Characteristic *gatt.Characteristic
}

type Bluetooth struct {
	in         <-chan []byte
	connection chan bluetoothConnection
}

func NewBluetooth(in <-chan []byte) *Bluetooth {
	return &Bluetooth{in: in, connection: make(chan bluetoothConnection)}
}

func (this *Bluetooth) Start() {
	go this.writer()

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
		select {}
	}
}

func (this *Bluetooth) writer() {
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
			// TODO deal with call blocking on connection close
			connection.Periph.WriteCharacteristic(connection.Characteristic, msg, true)
		}
	}
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

			rxFound := false
			for _, c := range cs {
				//if c.UUID().Equal(uartServiceTXCharID) {
				//log.Println("TX Characteristic found")

				//periph.DiscoverDescriptors(nil, c)
				//periph.SetNotifyValue(c, func(c *gatt.Characteristic, b []byte, err error) {
				//log.Printf("Got back '%s'\n", string(b))
				//})

				if c.UUID().Equal(uartServiceRXCharID) {
					rxFound = true
					fmt.Println("Connected to copter on bluetooth")
					this.connection <- bluetoothConnection{periph, c}
				}
			}
			if !rxFound {
				fmt.Println("RX characteristic not found")
				return
			}
		}
	}
}

func (this *Bluetooth) onDisconnected(periph gatt.Peripheral, err error) {
	fmt.Println("Disconnected from copter on bluetooth")
	periph.Device().Scan(nil, false)
}
