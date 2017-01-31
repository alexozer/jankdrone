package client

import (
	"fmt"
	"log"
	"math"

	"github.com/alexozer/jankcopter/shm"
	"github.com/golang/protobuf/proto"
)

type Sender struct {
	bluetooth  *Bluetooth
	varsIn     <-chan []BoundVar
	varsOut    chan<- BoundVar
	encodedIn  chan []byte
	encodedOut chan [][]byte

	status chan string
}

func NewSender(varsIn <-chan []BoundVar, varsOut chan<- BoundVar, status chan string) *Sender {
	encodedIn, encodedOut := make(chan []byte), make(chan [][]byte)
	return &Sender{
		NewBluetooth(encodedOut, encodedIn, status),
		varsIn, varsOut,
		encodedIn, encodedOut,
		status,
	}
}

func (this *Sender) Start() {
	this.bluetooth.Start()
	go this.write()
	go this.read()
}

func (this *Sender) write() {
	for varSlice := range this.varsIn {

		var out [][]byte
		for _, v := range varSlice {
			msg := new(shm.ShmMsg)
			newTag := int32(v.Tag)
			msg.Tag = &newTag

			if v.Value != nil {
				switch value := v.Value.(type) {
				case int:
					msg.Value = &shm.ShmMsg_IntValue{IntValue: int32(value)}
				case float64:
					msg.Value = &shm.ShmMsg_FloatValue{FloatValue: float32(value)}
				case bool:
					msg.Value = &shm.ShmMsg_BoolValue{BoolValue: value}
				default:
					log.Fatal("Unexpected shm variable type")
				}
			}

			encodedVar, err := proto.Marshal(msg)
			if err != nil {
				log.Fatal("Failed to encode shm message:", err)
			}
			if len(encodedVar) > math.MaxUint8 {
				this.status <- "Failed to send encoded variable; length too long"
				continue
			}
			framedVar := append([]byte{byte(len(encodedVar))}, encodedVar...)
			out = append(out, framedVar)
		}

		this.encodedOut <- out
	}
}

func (this *Sender) read() {
	for encodedVar := range this.encodedIn {
		shmMsg := new(shm.ShmMsg)
		if proto.Unmarshal(encodedVar[1:], shmMsg) != nil {
			this.status <- "Unable to unmarshal remote message"
			continue
		}

		var outValue interface{}
		switch inValue := shmMsg.Value.(type) {
		case *shm.ShmMsg_IntValue:
			outValue = int(inValue.IntValue)
		case *shm.ShmMsg_FloatValue:
			outValue = float64(inValue.FloatValue)
		case *shm.ShmMsg_BoolValue:
			outValue = inValue.BoolValue
		default:
			this.status <- "Unknown remote var type"
			continue
		}

		v, err := BindVarTag(int(*shmMsg.Tag), outValue)
		if err != nil {
			this.status <- fmt.Sprint("Failed to bind remote var:", err)
			continue
		}
		this.varsOut <- v
	}
}
