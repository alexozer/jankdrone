#!/usr/bin/env python2

from shm_update_pb2 import ShmUpdate
import serial

update = ShmUpdate()

desire1 = update.var.add()
desire1.tag = 1
desire1.intValue = 127

desire2 = update.var.add()
desire2.tag = 2
desire2.intValue = 255

enable_controller = update.var.add()
enable_controller.tag = 5
enable_controller.boolValue = True

wrongType = update.var.add()
wrongType.tag = 10
wrongType.floatValue = 234

print update

ser = serial.Serial('/dev/ttyACM0')
ser.write(update.SerializeToString())
ser.flush()
ser.close()
