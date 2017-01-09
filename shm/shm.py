from collections import namedtuple

Var = namedtuple('Var', ['value', 'tag'])

untagged_shm = {
    'switches': {
        'softKill': True,
        'calibrate': False,
    },

    'thrusters': {
        'front': 0.0,
        'left': 0.0,
        'right': 0.0,
        'back': 0.0,
    },

    'controller': {
        'enabled': False,
        'verticalForce': 0.0,

        'yawEnabled': True,
        'yawDesire': 0.0,
        'yawP': 1.0,
        'yawI': 0.0,
        'yawD': 0.0,

        'pitchEnabled': True,
        'pitchDesire': 0.0,
        'pitchP': 1.0,
        'pitchI': 0.0,
        'pitchD': 0.0,

        'rollEnabled': True,
        'rollDesire': 0.0,
        'rollP': 1.0,
        'rollI': 0.0,
        'rollD': 0.0,
    },

    'leds': {
        'front': 0,
        'left': 0,
        'right': 0,
        'back': 0,
    },

    'placement': {
        'yaw': 0.0,
        'pitch': 0.0,
        'roll': 0.0,
        'altitude': 0.0,
    },

    'temperature': {
        'gyro': 0.0,
    },

    'power': {
        'voltage': 0.0,
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
