#!/bin/bash

protoc --plugin=protoc-gen-nanopb=../drone/lib/nanopb/generator/protoc-gen-nanopb \
	--nanopb_out=../drone/src --nanopb_out=../handheld/src shm.proto
protoc -I=. --go_out=. shm.proto
./generate_shm.py
