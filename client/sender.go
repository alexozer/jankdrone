package client

import (
	"log"

	"github.com/alexozer/jankcopter/shm"
	"github.com/golang/protobuf/proto"
)

type Sender struct {
	bluetooth *Bluetooth
	vars      <-chan []BoundVar
	encoded   chan []byte
}

func NewSender(vars <-chan []BoundVar) *Sender {
	encoded := make(chan []byte)
	return &Sender{NewBluetooth(encoded), vars, encoded}
}

func (this *Sender) Start() {
	go this.bluetooth.Start()

	for varSlice := range this.vars {

		shmMsg := new(shm.ShmMsg)
		for _, v := range varSlice {
			protoVar := new(shm.ShmMsg_Var)
			newTag := int32(v.Tag)
			protoVar.Tag = &newTag

			switch value := v.DefaultValue.(type) {
			case int:
				newValue := int32(value)
				protoVar.IntValue = &newValue
			case float64:
				newValue := float32(value)
				protoVar.FloatValue = &newValue
			case bool:
				protoVar.BoolValue = &value
			default:
				log.Fatal("Unexpected shm variable type")
			}

			shmMsg.Var = append(shmMsg.Var, protoVar)
		}

		out, err := proto.Marshal(shmMsg)
		if err != nil {
			log.Fatal("Failed to encode shm message:", err)
		}
		this.encoded <- out
	}
}
