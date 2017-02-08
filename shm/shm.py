from collections import namedtuple

Var = namedtuple('Var', ['value', 'tag'])

# Shm (shared memory) is a lightweight database of primitive datatypes designed
# for message passing between code modules, devices, and users. Shm groups have
# a name and contain a set of shm variables, each with a name, type, and default
# value. The type of a shm variable is the type of its default value.
#
# The drone uses a right-handed coordinate system with X pointing right, Y
# pointing forward, and Z pointing up.

untagged_shm = {
    'switches': {
        'softKill': True,
        'calibrateEscs': False,
        'calibrateImu': False,
        'calibrateAltimeter': False,
    },

    # Specified from 0 degrees and moving counterclockwise
    # More vars may be added as necessary to satisfy thruster count constant,
    # and extra vars are okay
    'thrusters': {
        't0': 0.0,
        't1': 0.0,
        't2': 0.0,
        't3': 0.0,
        't4': 0.0,
        't5': 0.0,
        't6': 0.0,
        't7': 0.0,
    },

    'desires': {
        'force': 0.0,
        'altitude': 0.0,
        'yaw': 0.0,
        'pitch': 0.0,
        'roll': 0.0,

        'xVel': 0.0,
        'yVel': 0.0,
        'zVel': 0.0,
        'yawVel': 0.0,
        'pitchVel': 0.0,
        'rollVel': 0.0,
    },

    'controller': {
        'enabled': False,
    },

    'controllerOut': {
        'altitude': 0.0,
        'yaw': 0.0,
        'pitch': 0.0,
        'roll': 0.0,
    },

    'placement': {
        'altitude': 0.0,
        'yaw': 0.0,
        'pitch': 0.0,
        'roll': 0.0,
    },

    'temperature': {
        'gyro': 0.0,
        'altimeter': 0.0,
    },

    'power': {
        'voltage': 0.0,
        'low': False,
        'critical': False,
    },

    'led': {
        'pattern': 1, # 1 == dynamic pattern
        'brightness': 32,
    },

    'deadman': {
        'enabled': True,
        'maxTilt': 30.0,
    },

    'remote': {
        'connected': False,
    },

    'threadTime': {
        'thrust': 0,
        'remote': 0,
        'imu': 0,
        'controller': 0,
        'led': 0,
        'altimeter': 0,
    },

    'altitudeConf': {
        'enabled': False,
        'p': 0.001,
        'i': 0.0,
        'd': 0.0,
    },

    'yawConf': {
        'enabled': False,
        'p': 0.001,
        'i': 0.0,
        'd': 0.0,
    },

    'pitchConf': {
        'enabled': False,
        'p': 0.01,
        'i': 0.0,
        'd': 0.0,
    },

    'rollConf': {
        'enabled': False,
        'p': 0.01,
        'i': 0.0,
        'd': 0.0,
    },
}

def tag(untagged):
    current_tag = 0
    tagged_groups = {}
    for g_name, g_vars in sorted(untagged.items()):
        tagged_vars = {}
        for v_name, v_value in sorted(g_vars.items()):
            tagged_vars[v_name] = Var(v_value, current_tag)
            current_tag += 1

        tagged_groups[g_name] = tagged_vars

    return tagged_groups

shm = tag(untagged_shm)
