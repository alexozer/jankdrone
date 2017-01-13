# jankcopter
The jankest quadcopter the world has ever seen.

# Dependencies
- platformio
- Python 3
- Go
- Protobuf
	- [Go](https://github.com/golang/protobuf)
	- [Python](https://github.com/google/protobuf/tree/master/python)

# TODO

## Software
- [x] Shared memory
- [x] Threading
- [x] Logging
- Bluetooth message passing
	- [x] Protocol
	- [x] Server
	- Client
		- [x] Handheld controller
		- [x] CLI
	- [ ] Deadman (softkill / land on remote disconnect)
- [x] IMU readings
- [x] Controller
- Thrust writer
	- [x] Basic linear writing
	- [x] Calibration
	- [ ] Force-thrust function from bollard-poll
- Voltage monitor
	- [x] Measurement
	- [ ] Low voltage action
- [ ] LEDs
- Autonomous
	- [ ] Mission framework
	- [ ] Missions

## Electrical
- Main board
	- [x] Teensy mount
	- [x] MPU9250 mount
	- [x] Bluefruit mount
	- [x] ESC plugs
	- [x] Voltage monitor wire
	- [ ] LEDs
- Power board
	- [ ] 12V power rail
		- Needs rework to support high current
	- [x] Voltage measurement source
	- [x] 5V-regulated power for computer board

## Mechanical
- [x] PVC frame (failed, too heavy)
- [x] New metal sheet only frame
- [x] Thrusters
- [x] Landing posts
