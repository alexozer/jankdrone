> *jank*: broken, meaningless, ridiculously moronic, or of questionable quality

# jankdrone
The jankest autonomous drone ever built and programmed from scratch. Truly insulting to drones in general.

[Hacker News Thread](https://news.ycombinator.com/item?id=19354431)

[Photo Album](https://photos.app.goo.gl/QhSZEyd4DA1r9S9G6)

[Watch Jankdrone Fly! (and crash)](https://www.youtube.com/watch?v=-ZaXxxx3Wac)

[![jankdrone](/doc/video-thumbnail.png)](https://www.youtube.com/watch?v=-ZaXxxx3Wac)

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
	- [x] ~~Altimeter mount~~ broken altimeter
- Power board
	- [x] 12V power rail
	- [x] Voltage measurement source
	- [x] 5V-regulated power for computer board
	- [x] 3.3V-regulated power for radio
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

## My description on HN from 2019

I really ought to have a blog post detailing more of this, but here's a little backstory anyway. My best friend from high school and I wanted to experiment with creating our own modular drones, and although we bought a bunch of parts, we ended up leaving for college before we could do anything. A couple years later I had some time on my hands, and I decided that I wanted to see whether or not I could actually build and program a drone myself. For the project, I tried to focus on writing high quality software while still managing to build something flyable with my nearly-nonexistent mechanical skills. Even though much of this project was physical (and electrical), I still largely consider this a software project, actually.

Physically, my prototypes are something you might laugh at. Zip ties, styrofoam, and Gorilla tape were my go-to materials for the most part. I originally started by mounting (read: zip-tying and taping) components to PVC tubes and metal sheets; combined with the pretty large LiPo battery I bought, it should have been no wonder that four motors were no match for the weight of the thing. I later decided to screw together 6 strips of carbon fiber sheet (a "frame") and add two motors to help assure my drone can actually lift greater than two inches off the ground at a time. Reworking the flight controller to work with six motors instead of just two was a fun challenge; I decided to make the flight controller support an unlimited number of motors in a circle while I was at it.

Figuring out the optimal wireless technology for the drone was a bit of a process as well. It was frustrating when I thought I had everything ready for a test flight, but then slowly realized that, perhaps, I wasn't going to be able to get away with using Bluetooth LE over non-trivial distances. Eventually I found a radio module with superb distance and transmission rate; and after some struggle, I got them working. You can see in the video that I could control the drone from quite far away!

It took quite a few days of test "flights" to get the drone to fly as well as it did in the video (if you consider the oscillating nightmare in the video "flying well"). Even getting the drone to launch two feet in the air, instantly lose control and backflip, and crash back down on the ground felt like an AMAZING feat from my perspective; it demonstrated all my systems were at least online and functional. I slowly increased the hover time of each attempt by a couple seconds or so, adjusting my controller tuning and implementation as I went. The flight in the video was nowhere near as primitive as some of my early experiments, even.
