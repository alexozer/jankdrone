#!/bin/bash

../copter/lib/nanopb/generator-bin/protoc --nanopb_out=../copter/src shm.proto
protoc -I=. --go_out=. shm.proto
./generate_shm.py
