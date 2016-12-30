#!/bin/bash

../copter/lib/nanopb/generator-bin/protoc --nanopb_out=../copter/src shm_update.proto
./generate_shm.py
