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
- [x] IMU readings
- Controller
	- [x] Quadcopter support
	- [ ] Hexcopter (or 2n-copter) support
- Thrust writer
	- [x] Basic linear writing
	- [x] Calibration
	- [ ] Force-thrust function from bollard-poll
- [x] Voltage measurement
- [ ] LED strips
- [ ] Deadman (kill / land on disconnection, critical battery)
- Autonomous
	- [x] Mission framework
	- [ ] Missions

## Electrical
- Main board
	- [x] Teensy mount
	- [x] MPU9250 mount
	- [x] Bluefruit mount
	- [x] ESC plugs
	- [x] Voltage monitor wire
	- [ ] LED strips
- Power board
	- [x] 12V power rail
	- [x] Voltage measurement source
	- [x] 5V-regulated power for computer board

## Mechanical
- Version 1 (failed, too heavy)
	- [x] PVC frame
	- [x] PVC tube thruster mounts
	- [x] Metal sheet electronics mount
- Version 2 (failed, too heavy, thrust blockage)
	- [x] Metal sheet only frame
	- [x] Thrusters
	- [x] Landing posts
- Version 3
	- [ ] Carbon fiber frame
