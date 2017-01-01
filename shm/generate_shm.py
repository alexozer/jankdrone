#!/usr/bin/env python3

from pyratemp import pyratemp
import shm

templates = [
    ('../copter/src/shmdef.cpp.template', '../copter/src/shmdef.cpp'),
    ('../client/shmdef.go.template', '../client/shmdef.go'),
]

for t in templates:
    pt = pyratemp.Template(filename=t[0])
    with open(t[1], 'w') as out:
        out.write(pt(shm=shm.shm))
