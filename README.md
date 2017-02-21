> *jank*: broken, meaningless, ridiculously moronic, or of questionable quality

# jankdrone
The jankest autonomous drone ever built and programmed from scratch. Truly insulting to drones in general.

**Watch jankdrone fly! (and crash)**

[![jankdrone](https://img.youtube.com/vi/-ZaXxxx3Wac/0.jpg)](https://www.youtube.com/watch?v=-ZaXxxx3Wac)

# Dependencies
- platformio
- Python 3
- Go
- Protobuf
	- [Go](https://github.com/golang/protobuf)
	- [Python](https://github.com/google/protobuf/tree/master/python)

# TODO

## Software
- [x] Shared memory (SHM)
- [x] Threading
- [x] Logging
- SHM message passing
	- [x] Protocol
	- [x] Server
	- [x] Serial I/O
	- [x] Bluetooth I/O (failed, signal too weak)
	- [x] Radio I/O
	- [x] Client terminal status GUI
	- [X] Client command REPL
	- [x] Handheld serial to radio map
	- [x] Handheld control desire serialization
- Flight controller (hexcopter / 2n-copter)
	- Absolute controls
		- [x] Force
		- [x] Yaw
		- [x] Pitch
		- [x] Roll
		- [ ] Altitude
		- [ ] Latitude and longitude
	- Velocity controls
		- [x] Yaw
		- [x] Pitch
		- [x] Roll
		- [ ] X and Y
		- [ ] Z (altitude)
- Thrust writer
	- [x] Basic linear writing
	- [x] Calibration
	- [ ] Force-thrust function from bollard-poll
- [x] Voltage measurement
- [x] LED strips
- [x] Deadman (kill / land in critical conditions)
- Autonomous
	- [x] Mission framework
	- [ ] Missions

## Electrical
- Main board
	- [x] Teensy mount
	- [x] MPU9250 mount
	- [x] Bluefruit mount (useless now, bluetooth too weak)
	- [x] ESC plugs
	- [x] Voltage monitor wire
	- [x] LED strips
	- [x] RFM69 Radio transceiver
- Power board
	- [x] 12V power rail
	- [x] Voltage measurement source
	- [x] 5V-regulated power for computer board
- Handheld controller
	- [x] Joysticks
	- [x] Softkill switch
	- [x] Radio tranceiver

## Mechanical

### Drone
- Version 1 (failed, too heavy)
	- [x] PVC frame
	- [x] 4 PVC tube thrusters and mounts
	- [x] Metal sheet electronics mount
- Version 2 (failed, too heavy, thrust blockage)
	- [x] Metal sheet only frame
	- [x] 4 thrusters and mounts
	- [x] Landing posts
- Version 3
	- [x] Carbon fiber frame
	- [x] 6 thrusters and mounts
	- [x] LED strips
	- [x] Tetrahedral-ish electronics shell
	- [x] DJI Phantom-style props that don't vibrate like crazy

### Handheld
- Version 1 (too cramped)
	- [x] Gutted Xbox controller for old Arduino Nano
	- [x] 2 joysticks
	- [x] Softkill toggle switch
- Version 2
	- [x] Old acryllic Raspberry Pi case
	- [x] Mounting protoboard
	- [x] 2 joysticks with embedded buttons
	- [x] Radio tranceiver
	- [x] External power and regulator
