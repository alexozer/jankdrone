package client

import (
	"fmt"
	"log"
	"math"

	"github.com/alexozer/jankdrone/shm"
	"github.com/golang/protobuf/proto"
)

type Sender struct {
	varsIn  <-chan []BoundVar
	varsOut chan<- BoundVar
	status  chan string

	handheld, drone *Serial
}

func NewSender(varsIn <-chan []BoundVar, varsOut chan<- BoundVar, status chan string) *Sender {
	return &Sender{
		varsIn, varsOut,
		status,
		NewSerial("/dev/ttyUSB0", 115200, status),
		NewSerial("/dev/ttyACM0", 115200, status),
	}
}

func (this *Sender) Start() {
	this.handheld.Start()
	this.drone.Start()

	encodedInChan := make(chan chan []byte)
	encodedOutChan := make(chan chan [][]byte)
	go this.read(encodedInChan)
	go this.write(encodedOutChan)

	go func() {
		var handheldConnected, droneConnected bool

		for {
			select {
			case handheldConnected = <-this.handheld.Connected:
			case droneConnected = <-this.drone.Connected:
			}

			// Prefer drone connection over handheld since it shouldn't be
			// wireless
			var selectedSerial *Serial
			if droneConnected {
				selectedSerial = this.drone
			} else if handheldConnected {
				selectedSerial = this.handheld
			} else {
				// Nothing connected
				continue
			}

			encodedInChan <- selectedSerial.Out
			encodedOutChan <- selectedSerial.In
		}
	}()
}

func (this *Sender) write(encodedOutChan chan chan [][]byte) {
	encodedOut := <-encodedOutChan
	for {
		select {
		case encodedOut = <-encodedOutChan:
		case varSlice := <-this.varsIn:

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

			sz := 0
			for _, s := range out {
				sz += len(s)
			}
			encodedOut <- out
		}
	}
}

func (this *Sender) read(encodedInChan chan chan []byte) {
	encodedIn := <-encodedInChan
	for {
		select {
		case encodedIn = <-encodedInChan:
		case encodedVar := <-encodedIn:
			for len(encodedVar) > 0 {
				shmMsg := new(shm.ShmMsg)
				if int(encodedVar[0]) > len(encodedVar[1:]) {
					this.status <- "Received message length exceeds buffer length"
					break
				}
				if proto.Unmarshal(encodedVar[1:encodedVar[0]+1], shmMsg) != nil {
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
				encodedVar = encodedVar[encodedVar[0]+1:]
			}
		}
	}
}
