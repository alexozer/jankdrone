#!/bin/bash

protoc --plugin=protoc-gen-nanopb=../copter/lib/nanopb/generator/protoc-gen-nanopb \
	--nanopb_out=../copter/src --nanopb_out=../handheld/src shm.proto
protoc -I=. --go_out=. shm.proto
./generate_shm.py
